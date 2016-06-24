#ifndef BALL_H
#define BALL_H

#include <QVector2D>

class Ball
{
public:
    Ball(double r, double mass, double speed, QVector2D p, QVector2D v);
    Ball():Rad{0},Mass{0},Speed{0}{}
    double Rad;
    double Mass;
    double Speed;

    QVector2D pos;
    QVector2D V;

};


#endif // BALL_H
