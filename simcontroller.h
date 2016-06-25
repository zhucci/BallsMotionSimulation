#ifndef SIMCONTROLLER_H
#define SIMCONTROLLER_H

#include <qvector.h>
#include <QVector2D>
#include "ball.h"

typedef double time_;


struct CollisionEvent{
    time_ tau;
    int b1;
    int b2;
};


class SimController
{
public:

    SimController(int xs, int ys, double deltaT);

    int SetInitState();

    int Move();

    const QVector<Ball> &GetBalls() const;

    int ClearScene();

    int SetMaxStepLength(double stepL);

    int AddBall(Ball &ball);

    bool IsInitialize(){return (bool)balls.size();}

    double GetTime(){return tau;}

private:
    int initBallsScene();
    int initFutureEvents();
    int updateFutureCollidesAfterEvent(CollisionEvent event);
    int processCollision(CollisionEvent event);
    inline void nextStateAfter(time_ delta_tau);
    bool findCollisionEvent(int forball, CollisionEvent &event);
    inline void nextCollisionUpdate();
    bool InArea(QVector2D pos);
    bool findCollisionTime(int b1, int b2, time_ &time);

private:

    time_ tau;

    time_ delta_tau;
    int stepSize;
    CollisionEvent nextCollision;

    QVector2D minP, maxP;

    QVector<Ball> balls;

    //ball number N will collide at events[N].tau time_
    QVector<CollisionEvent> events;
};

#endif // SIMCONTROLLER_H
