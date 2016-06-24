#include "gui.h"
#include "ui_gui.h"
#include "connectdialog.h"
#include <QMessageBox>
GUI::GUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::GUI)
{
    ui->setupUi(this);
    simulator = new BallMotionSim(this);
    simulator->show();
    simulator->InitSimulation();
    on_serverAddressChanged(simulator->GetServerAddress());

}
void GUI::ErrorMessageShow(QString err){
    QMessageBox *mes = new QMessageBox(this);
    mes->setWindowTitle("Error");
    mes->setText(err);
    mes->show();
}
void GUI::on_serverAddressChanged(QString addr){

        QLabel *addrLabel = new QLabel(this);
        addrLabel->setText(addr);
        statusBar()->addPermanentWidget(addrLabel);
}

void GUI::on_actionConnect_triggered(){

    ConnectDialog *dialog = new ConnectDialog(this);
    dialog->exec();
    if(dialog->IsAccepted()){
        auto addr = dialog->GetAddres();
        simulator->ConnectToServer(addr.first,addr.second);
    }

}

void GUI::on_actionStartSim_triggered(){

    simulator->StartSim();

}

void GUI::on_actionStopSim_triggered(){

    simulator->StopSim();
}

void GUI::on_actionReset_triggered(){

    simulator->ResetBalls();

}

GUI::~GUI()
{
    delete ui;
}
