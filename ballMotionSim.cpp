#include "ballMotionSim.h"
#include <QApplication>
#include <QBrush>
#include "synchronizationmanager.h"
#include <QDataStream>
#include <QMessageBox>

BallMotionSim::BallMotionSim(QWidget *parent):QWidget(parent),
    status(SynchroStatus::Stop),ttl(3),StepPause(50)
{ 
    setMouseTracking(true);
    this->setCursor(Qt::CrossCursor);
    this->move(30,50);
    setPalette(QPalette(QColor(245, 245, 245)));
    setAutoFillBackground(true);

    timer = new QBasicTimer();

    syncManager = new SynchronizationManager(this);

    connect(syncManager, SIGNAL(ReadData(QByteArray)),this,SLOT(ParseMessageFrom(QByteArray)));
    connect(syncManager, SIGNAL(ErrorOccure(QString)),parent,SLOT(ErrorMessageShow(QString)));



}
int BallMotionSim::InitSimulation(){

   try{
       this->setFixedHeight(450);
       this->setFixedWidth(650);
       this->controller = new SimController(width(),height(),4);
       controller->SetInitState();

       return EXIT_SUCCESS;
   }
   catch(...){
        return EXIT_FAILURE;
    }
}

int BallMotionSim::StartSim(bool synchro){

    if(status!=SynchroStatus::Start){
        status = SynchroStatus::Start;

        //Init controller
        if(!controller->IsInitialize())
             controller->SetInitState();

        //Start timer
        timer->start(StepPause,this);

        //Synchronization
        if(synchro){
             status = SynchroStatus::Start;
             QByteArray data = headerOfMessage(status);
             syncManager->SyncData(data);
        }
    }

    return EXIT_SUCCESS;
}

void BallMotionSim::StopSim(bool synchro){

    if(status!=SynchroStatus::Stop){

         status = SynchroStatus::Stop;
         timer->stop();

         if(synchro){
            auto data = headerOfMessage(status);
            syncManager->SyncData(data);
         }
    }

}

int BallMotionSim::ResetBalls(bool synchro){

    if(status!=SynchroStatus::Reset){

        StopSim(false);
        status = SynchroStatus::Reset;
        controller->SetInitState();
        this->update();

        if(synchro){
            auto data = headerOfMessage(status);
            syncManager->SyncData(data);
        }
    }
    return EXIT_SUCCESS;
}

void BallMotionSim::AddNewBalls(QVector<Ball> balls){

    for(auto &b : balls){
        if(!IsBallsCollide(b.pos,b.Rad))
            controller->AddBall(b);
    }
    this->update();
}

void BallMotionSim::newBallAdded(Ball ball){

    //add new ball to simulation module

    QVector<Ball> newballs;

    newballs.push_back(ball);

    auto data = packBalls(newballs);

    syncManager->SyncData(data);

    controller->AddBall(ball);

    this->update();
}

void BallMotionSim::timerEvent(QTimerEvent *){

        controller->Move();
        this->update();
        timer->stop();
        timer->start((int)(StepPause*controller->NextDeltaT()),this);
}

void BallMotionSim::paintEvent(QPaintEvent *){

    updateImage();
}

void BallMotionSim::updateImage(){

    const QVector<Ball> &balls = controller->GetBalls();

    QPainter paiter(this);
    paiter.drawRect(0,0,this->width()-3,this->height()-3);

    QPen pen(Qt::Dense3Pattern,1,Qt::SolidLine);
    pen.setColor(Qt::black);
    pen.setWidth(3);
    paiter.setPen(pen);

    pen.setColor(Qt::red);
    paiter.setPen(pen);

    for(Ball b : balls){
       paiter.drawEllipse((int)b.pos.x()-(int)b.Rad,(int)b.pos.y()-(int)b.Rad,2*(int)b.Rad,2* (int)b.Rad);
    }

}


void BallMotionSim::StatusChanged(int ){

}

int BallMotionSim::ConnectToServer(QHostAddress address, int port){

    QByteArray message = headerOfMessage(SynchroStatus::GetBalls);

    controller->ClearScene();

    if(syncManager->ConnectToPeer(address,port,message)){
        QMessageBox *mes = new QMessageBox(this);
        mes->setText(QString("Can't connect to %1 : %2").arg(address.toString()).arg(port,7));
        mes->show();
    }
    return EXIT_SUCCESS;
}
QByteArray BallMotionSim::headerOfMessage(SynchroStatus newStatus){

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);

    quint16 port = syncManager->GetServerPort();
    quint16 status = newStatus;
    quint32 ip = syncManager->GetServerIP();

    out<<(quint32)ip<<(quint16)port<<(quint16)ttl<<(quint16)status;

    return data;
}

void BallMotionSim::ParseMessageFrom(QByteArray byteArray){

    QDataStream str(byteArray);
    quint32 ip;
    quint16 port;
    quint16 newStatus;
    quint16 mesTTL;
    str>>ip;
    str>>port;
    str>> mesTTL;
    str>> newStatus;

    //message time to live
    if(!(--mesTTL))
        return;
    else
        byteArray[7]=mesTTL;

    //forbidden ip
    if(!ip)
        return;

    //messege loop
    if(ip==syncManager->GetServerIP() && port == syncManager->GetServerPort())
        return;


    QHostAddress peerAddress(ip);
    tcpAddr senderOfMessage(peerAddress,port);

    if(!syncManager->HasSuchAddress(senderOfMessage)){
        syncManager->AddNewAddressToRootTable(senderOfMessage);
    }


    if(status != newStatus && newStatus==SynchroStatus::Start){
        //Broadcast same message with --ttl
        syncManager->SyncData(byteArray);

        StartSim(false);

    }
    else if(status != newStatus && newStatus == SynchroStatus::Stop){
        //Broadcast same message with --ttl
        syncManager->SyncData(byteArray);

        StopSim(false);
    }
    else if(status != newStatus && newStatus == SynchroStatus::Reset){

        //Broadcast same message with --ttl
        syncManager->SyncData(byteArray);

        StopSim(false);
        ResetBalls(false);
    }
    else if(newStatus == SynchroStatus::GetBalls){

        QByteArray colOfBall= packBalls(controller->GetBalls());

        //send response
        auto data = headerOfMessage(SynchroStatus(status));
        syncManager->ConnectToPeer(senderOfMessage.host, senderOfMessage.port, data);
        syncManager->ConnectToPeer(senderOfMessage.host, senderOfMessage.port, colOfBall);
    }
    else if(newStatus == SynchroStatus::AddBalls){
        QVector<Ball> ballRecv = unPackBalls(byteArray);
        AddNewBalls(ballRecv);
        syncManager->SyncData(byteArray);
    }
}


QByteArray BallMotionSim::packBalls(const QVector<Ball> &ballArray){


    QByteArray ballsRaw = headerOfMessage(SynchroStatus::AddBalls);

     QDataStream out(&ballsRaw, QIODevice::Append);

    //Data
    out<<(quint16) ballArray.size();

    for(int i=0;i<ballArray.size();i++){
        out<< (double) (ballArray[i].Mass);
        out<< (double) ballArray[i].Rad;
        out<< (double) ballArray[i].Speed;
        out<< (double) ballArray[i].pos.x()<<(double) ballArray[i].pos.y();
        out<< (double) ballArray[i].V.x()<<(double) ballArray[i].V.y();
    }
    return ballsRaw;
}
QVector<Ball> BallMotionSim::unPackBalls(QByteArray &data){

    QDataStream in(data);

    quint16 ballsAmount;

    in>>ballsAmount;
    in>>ballsAmount;
    in>>ballsAmount;
    in>>ballsAmount;
    in>>ballsAmount;
    in>>ballsAmount;

    double M,R,S,x,y,Vx,Vy;

    QVector<Ball> ballCollection;

    for(int i=0;i < ballsAmount;i++){
        Ball ball;

        in>>M>>R>>S>>x>>y>>Vx>>Vy;

        ball.Mass=M;
        ball.Rad = R;
        ball.Speed = S;
        ball.pos= {(float)x,(float)y};
        ball.V = {(float)Vx,(float)Vy};

        ballCollection.push_back(ball);
    }
    return ballCollection;
}

void BallMotionSim::mousePressEvent(QMouseEvent *click){ //add balls menu

    QPoint pos = click->pos();
    //check possition correctness

    BallCreator *bc = new BallCreator(QVector2D(pos),this);
    bc->exec();
    if(bc->Accepted()){
        Ball ball = bc->GetBall();
        if(!IsBallsCollide( ball.pos,ball.Rad))
            newBallAdded(ball);
    this->update();
    }
}

bool BallMotionSim::IsBallsCollide(QVector2D pos, double rad){
    auto &balls = controller->GetBalls();
    for(Ball ball : balls){
        if((ball.pos-pos).length() < (ball.Rad + rad))
            return true;
    }
    return false;
}
