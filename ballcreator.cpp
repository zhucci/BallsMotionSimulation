#include "ballcreator.h"


BallCreator::BallCreator(QVector2D pos, QWidget *parent) : QDialog(parent){
    dialog.setupUi(this);
    ball.pos=pos;
    IsDone = false;
}
BallCreator::~BallCreator(){

}

 void BallCreator::on_OK_button_pressed(){

     ball.Mass=dialog.ballMassValue->value();
     ball.Rad=dialog.ballRadiusValue->value();
     ball.V = {(float) dialog.ballVx->value(),(float) dialog.ballVy->value()};
     ball.Speed = dialog.ballSpeedValue->value();
     ball.V *= ball.Speed;

     IsDone = true;

     this->close();
 }
 void BallCreator::on_cancel_button_pressed(){
    this->close();
 }
