#include "ballMotionSim.h"
#include <QApplication>
#include <QBrush>
#include "synchronizationmanager.h"
#include <QDataStream>
#include <QMessageBox>

BallMotionSim::BallMotionSim(QWidget *parent):QWidget(parent),
    status(SynchroStatus::Stop),prevMessageDesc(0)
{ 
    setMouseTracking(true);
    this->setCursor(Qt::CrossCursor);
    this->move(30,50);
    setPalette(QPalette(QColor(245, 245, 245)));
    setAutoFillBackground(true);

    timer = new QBasicTimer();
    //this->adjustSize();
    syncManager = new SynchronizationManager(this);

    connect(syncManager, SIGNAL(ReadData(tcpAddr , QByteArray)),this,SLOT(ParseMessageFrom(tcpAddr,QByteArray)));
    connect(syncManager, SIGNAL(ErrorOccure(QString)),parent,SLOT(ErrorMessageShow(QString)));
    //connect(syncManager, SIGNAL(serverAddressChanged(QString)),parent,SLOT(on_serverAddressChanged(QString)));



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

    if(!showMustGoOn){
        if(!controller->IsInitialize())
            controller->SetInitState();
         timer->start(50,this);
         controller->Move();
         this->update();

         if(synchro){
             status = SynchroStatus::Start;
             auto data = statusChangedMessage(status);
             syncManager->SyncData(data);
         }
    }

    return EXIT_SUCCESS;
}

void BallMotionSim::StopSim(bool synchro){
    timer->stop();
     if(synchro){
        status = SynchroStatus::Stop;
        auto data = statusChangedMessage(status);
        syncManager->SyncData(data);
     }

}

int BallMotionSim::ResetBalls(bool synchro){

    controller->SetInitState();
    this->update();
    if(synchro){
        status = SynchroStatus::Reset;
        auto data = statusChangedMessage(status);
        syncManager->SyncData(data);
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
    //if ball don't collide
    //add new ball to simulation module

    QVector<Ball> balls;
    balls.push_back(ball);
    auto data = packBalls(balls);
    syncManager->SyncData(data);

    controller->AddBall(ball);
    this->update();
}

void BallMotionSim::timerEvent(QTimerEvent *){
        controller->Move();
        this->update();
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
    QByteArray array;
    QDataStream str(&array, QIODevice::WriteOnly);
    //Debug

    quint16 desc = ++prevMessageDesc;

    quint16 myPort = syncManager->GetServerPort();
    quint32 ip = address.toIPv4Address();
    str << (quint32) ip;                            // ip
    str << (quint16) myPort;                        // port
    str << (quint16) desc;                          //descriptor
    str << (quint16) SynchroStatus::GetBalls;       //status
                                                    //no data

    if(syncManager->ConnectToPeer(address,port,array)){
        QMessageBox *mes = new QMessageBox(this);
        mes->setText(QString("Can't connect to %1 : %2").arg(address.toString()).arg(port,7));
        mes->show();
    }
    return EXIT_SUCCESS;
}
QByteArray BallMotionSim::statusChangedMessage(SynchroStatus newStatus){

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);

    quint16 port = syncManager->GetServerPort();
    quint16 status = newStatus;
    quint16 descriptor = ++prevMessageDesc;
    quint32 ip = syncManager->GetServerIP();

    out<<(quint32)ip<<(quint16)port<<(quint16)descriptor<<(quint16)status;
    return data;
}

void BallMotionSim::ParseMessageFrom(tcpAddr mesSender, QByteArray byteArray){

    QDataStream str(byteArray);
    quint32 ip;
    quint16 port;
    quint16 descriptor;
    quint16 status;

    str>>ip;
    str>>port;
    str>> descriptor;
    str>> status;

    if(descriptor==prevMessageDesc)
        return;
    else
        prevMessageDesc = descriptor;


    if(status==SynchroStatus::Start){
        syncManager->SyncData(byteArray);
        StartSim(false);
    }
    else if(status == SynchroStatus::Stop){
        syncManager->SyncData(byteArray);
        StopSim(false);
    }
    else if(status == SynchroStatus::Reset){
        syncManager->SyncData(byteArray);
        StopSim(false);
        ResetBalls(false);
    }
    else if(status == SynchroStatus::GetBalls){
        QByteArray colOfBall= packBalls(controller->GetBalls());
        //send response
       syncManager->ConnectToPeer(mesSender.host, mesSender.port, colOfBall);
    }
    else if(status == SynchroStatus::AddBalls){
        QVector<Ball> ballRecv = unPackBalls(byteArray);
        AddNewBalls(ballRecv);
        syncManager->SyncData(byteArray);
    }
}

QByteArray BallMotionSim::packBalls(const QVector<Ball> &ballArray){
    QByteArray ballsRaw;
    QDataStream out(&ballsRaw, QIODevice::WriteOnly );
    quint32 ip = syncManager->GetServerIP();
    quint16 port = syncManager->GetServerPort();
    quint16 descriptor = ++prevMessageDesc;
    quint16 stat = SynchroStatus::AddBalls;
    //Header
    out<<(quint32) ip;
    out<<(quint16) port;
    out<<(quint16)descriptor;
    out<<(quint16)stat;
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
            return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
