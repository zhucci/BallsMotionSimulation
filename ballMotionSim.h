#ifndef BallMotionSim_H
#define BallMotionSim_H
#include <QWidget>

#include "simcontroller.h"
#include "ballcreator.h"
#include "synchronizationmanager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QBasicTimer>


class BallMotionSim : public QWidget
{
    Q_OBJECT
public:
    BallMotionSim(QWidget *parent);

    int InitSimulation();

    int StartSim(bool synchro=true);

    void StopSim(bool synchro=true);

    int ResetBalls(bool synchro=true);

    QString GetServerAddress(){QString str;syncManager->GetServerAddress(str);return str;}

    bool IsBallsCollide(QVector2D pos, double rad);

    void paintEvent(QPaintEvent *pe);

    int ConnectToServer(QHostAddress address, int port);



public slots:
    void timerEvent(QTimerEvent * event);
    void StatusChanged(int status);

    void newBallAdded(Ball ball);
    void AddNewBalls(QVector<Ball> balls);
    void ParseMessageFrom(QByteArray byteArray);
    void mousePressEvent(QMouseEvent *event);

private:
    void updateImage();
    QByteArray packBalls(const QVector<Ball> &balls);
    QVector<Ball> unPackBalls(QByteArray &data);
    QByteArray headerOfMessage(SynchroStatus newStatus);
private:
    SynchroStatus status;
    QBasicTimer *timer;
    quint16 ttl;
    int StepPause;
    SimController *controller;
    SynchronizationManager *syncManager;
};

#endif // BallMotionSim_H
