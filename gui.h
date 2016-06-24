#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include "ballMotionSim.h"
#include <QTimer>
#include "synchronizationmanager.h"
namespace Ui {
class GUI;
}

class GUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit GUI(QWidget *parent = 0);
    ~GUI();

public slots:

 void on_actionConnect_triggered();
 void on_actionStartSim_triggered();
 void on_actionStopSim_triggered();
 void on_actionReset_triggered();
 void ErrorMessageShow(QString);
 void on_serverAddressChanged(QString str);
private:
    Ui::GUI *ui;
    BallMotionSim *simulator;
};

#endif // GUI_H
