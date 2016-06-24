#ifndef BALLCREATOR_H
#define BALLCREATOR_H

#include <QDialog>

#include "ui_addballdialog.h"
#include "ball.h"

class BallCreator: public QDialog
{
    Q_OBJECT
public:
   BallCreator(QVector2D pos, QWidget *parent = 0);
    ~BallCreator();
   Ball &GetBall(){return ball;}
   bool Accepted(){return IsDone;}
signals:
    void ballInfoAcepted(Ball ball);
public slots:
   void on_OK_button_pressed();
   void on_cancel_button_pressed();
private:
    bool IsDone;
    Ui::AddBallDialog dialog;
    Ball ball;
};

#endif // BALLCREATOR_H
