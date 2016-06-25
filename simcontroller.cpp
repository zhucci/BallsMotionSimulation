#include "simcontroller.h"
#include <qmath.h>

SimController::SimController(int xs, int ys, double oneStepSize)
{
    minP = { 0,0};
    maxP = { (float) xs,(float) ys};
    stepSize=oneStepSize;

}
int SimController::SetInitState()
{
    tau = 0;
    initBallsScene();
    SetMaxStepLength(stepSize);
    initFutureEvents();
    nextCollisionUpdate();

    return EXIT_SUCCESS;
}

const QVector<Ball> &SimController::GetBalls() const{
    return balls;
}
int SimController::SetMaxStepLength(double stepL){

    double max_deltaT=45;
    for(Ball &b : balls){
        double dT = stepL / b.V.length();
        max_deltaT = dT < max_deltaT ? dT : max_deltaT;
    }
    delta_tau = max_deltaT;
    return EXIT_SUCCESS;
}

double SimController::NextDeltaT(){

    if ((tau + delta_tau ) < nextCollision.tau) {

        return 1;

    }
    else{

        double dt = nextCollision.tau - tau;
        return dt/delta_tau;
    }
}

int SimController::Move()
{
    if ((tau + delta_tau ) < nextCollision.tau) {
        tau += delta_tau;

        nextStateAfter(delta_tau);

    } else {

        double dt = nextCollision.tau - tau;
        tau += dt;

        nextStateAfter(dt);
        processCollision(nextCollision);
        //Equal speed different time
        //this->SetMaxStepLength(stepSize);

    }
    return EXIT_SUCCESS;
}
int SimController::AddBall(Ball &ball)
{
    balls.push_back(ball);

    CollisionEvent event;
    event.b1=balls.size()-1;
    event.b2=-1; //with wall
    event.tau=tau;
    //Add please for its events
    events.push_back(event);
    //Calculate next event
    updateFutureCollidesAfterEvent(event);
    //SetMaxStepLength(stepSize);

    return EXIT_SUCCESS;
}


int SimController::initBallsScene(){

    if(balls.size()){
        balls.clear();
    }
    //Ball 1
    Ball ball;
    ball.Mass = 400;
    ball.Rad = 40;
    ball.pos = { (float) 1*maxP.x()/3, (float) maxP.y()/2.};
    ball.Speed = 20;
    ball.V = {1,0};
    ball.V*=ball.Speed;
    balls.push_back(ball);
    //Ball 2
    ball.Mass =ball.Rad =30;
    ball.pos = { (float) 2*maxP.x()/3, (float) maxP.y()/2.};
    ball.Speed = 20;
    ball.V = {(float)-1,(float)0};
    ball.V.normalize();
    ball.V*=ball.Speed;
    balls.push_back(ball);
    //Ball 3
    ball.Mass =ball.Rad =30;
    ball.pos = {(float)1*maxP.x()/2,(float) maxP.y()/3.};
    ball.Speed = 50;
    ball.V = {(float)1,(float)-1};
    ball.V.normalize();
    ball.V*=ball.Speed;
    balls.push_back(ball);

    return EXIT_SUCCESS;
}

int SimController::ClearScene(){
    this->balls.clear();
    this->events.clear();
    return EXIT_SUCCESS;
}

int SimController::initFutureEvents(){

    CollisionEvent event;
    events.resize(balls.size());

    for(int i=0;i<balls.size();i++){

        if( findCollisionEvent(i,event)){
                events[i]=event;
        }
    }

    return EXIT_SUCCESS;
}

int SimController::updateFutureCollidesAfterEvent(CollisionEvent event){

    int b1 = event.b1;
    int b2 = event.b2;

    if(b1>=0 && findCollisionEvent(b1,event)){
        events[b1]=event;
    }
    if(b2>=0 && findCollisionEvent(b2,event)){
        events[b2]=event;
    }

    nextCollisionUpdate();

    return EXIT_SUCCESS;
}
int SimController::processCollision(CollisionEvent event){

    int &b1 = event.b1;
    int &b2 = event.b2;

    if(b1 >=0 && b2>=0){ //balls collision

        QVector2D dPos = balls[b2].pos - balls[b1].pos;

        dPos.normalize();

        double Vn1 = QVector2D::dotProduct(dPos,balls[b1].V);
        double Vn2 = QVector2D::dotProduct(dPos,balls[b2].V);

        QVector2D Vt1 = (balls[b1].V - Vn1 * dPos);
        QVector2D Vt2 = (balls[b2].V - Vn2 * dPos);

        double dM = balls[b1].Mass - balls[b2].Mass;
        double sumM =  balls[b1].Mass + balls[b2].Mass;

        double Vn1_ =  (dM * Vn1 + 2* balls[b2].Mass * Vn2)/sumM;
        double Vn2_ = (-dM * Vn2 + 2 * balls[b1].Mass * Vn1)/sumM;

        balls[b1].V =  Vn1_ * dPos + Vt1;
        balls[b2].V =  Vn2_ * dPos + Vt2;

        balls[b1].Speed = balls[b1].V.length();

        balls[b2].Speed = balls[b2].V.length();


    }
    else{ //with wall collision

        if(event.b2 == -1 || event.b2 == -3)
            balls[b1].V.setX((-1) * balls[b1].V.x());
        else
            balls[b1].V.setY((-1) * balls[b1].V.y());
    }

    updateFutureCollidesAfterEvent(event);

    return EXIT_SUCCESS;
}
inline void SimController::nextCollisionUpdate(){
    if(!balls.size())
        return;
    time_ tau_nc = events[0].tau;
    nextCollision = events[0];
    for(int i=1;i<events.size();i++){
         if(events[i].tau < tau_nc){
             tau_nc = events[i].tau;
             nextCollision = events[i];
         }
    }
}

void SimController::nextStateAfter(time_ delta)
{
    for(int i=0;i<balls.size();i++){
        balls[i].pos = balls[i].pos + balls[i].V*delta;
    }
}
 bool SimController::findCollisionEvent(int forBall, CollisionEvent &event){

     Ball &b = balls[forBall];

     double inf_dt = 0;
     bool isInit=false;
     int obstacle=0;

     //collision with walls
     double dt;
     if(b.V.x()>0){ //right wall
         inf_dt = (maxP.x()-b.Rad - b.pos.x()) / b.V.x();
         isInit=true;
         obstacle = -3;
     }
     else if(b.V.x()){//left wall
         inf_dt =  (-1)* (b.pos.x() - minP.x() - b.Rad) / b.V.x();
         obstacle = -1;
         isInit=true;
     }
     if(b.V.y()>0){//bottom wall
            dt  = (maxP.y()-b.Rad - b.pos.y())/ b.V.y();
          if((isInit && inf_dt > dt) || (!isInit && dt > 0)){
                inf_dt = dt;
                obstacle = -2;
                isInit=true;
          }
     }
     else if(b.V.y()){//top wall
            dt  = (-1)* ( b.pos.y() - minP.y()-b.Rad)/ b.V.y();
            if((isInit && inf_dt > dt) || (!isInit && dt > 0)){
                  inf_dt = dt;
                  obstacle = -4;
                  isInit=true;
            }
     }

     //collision with balls
     for(int i=0;i<balls.size() ;i++ ){
        if(i==forBall)
            continue;
        if(findCollisionTime(forBall,i,dt)){
            if((isInit && inf_dt > dt) || (!isInit && dt > 0)){
                inf_dt = dt;
                obstacle=i;
                isInit=true;
            }
        }
     }
     if(isInit){
         event.b1=forBall;
         event.b2 = obstacle;
         event.tau = tau + inf_dt;
         return true;
     }
    return false;
 }
bool SimController::findCollisionTime(int b1, int b2, time_ &time){

    QVector2D dP = balls[b1].pos - balls[b2].pos;
    QVector2D dV = balls[b1].V - balls[b2].V;
    double a = dV.lengthSquared();

    if(a < 1e-7) //a is real small value
        return false;

    double b = 2*dP.dotProduct(dP,dV);
    double c = dP.lengthSquared() - qPow(balls[b1].Rad + balls[b2].Rad,2);

    if(b>0)
        return false;

    double d = qPow(b,2) - 4*a*c;

    if(d>=0){
        double rootD = qPow(d,0.5);
        double dt1 = (-b + rootD) / (2 * a);
        double dt2 = (-b - rootD) / (2 * a);
        if(dt1>0 && dt2>0){
            time = dt1> dt2 ? dt2 : dt1;
            return true;
        }
        else{
            if (dt1>0){
                time = dt1;
                return true;
            }
            if(dt2>0){
                time = dt2;
                return true;
            }
        }
    }
    return false;
}
bool SimController::InArea(QVector2D pos){
    if(
        pos.x() < minP.x() ||
        pos.x() > maxP.x() ||
        pos.y() < minP.y() ||
        pos.y() > maxP.y()
      )
        return false;
    return true;
}
