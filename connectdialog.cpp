#include "connectdialog.h"
#include "ui_connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
    isAccepted = false;
}

ConnectDialog::~ConnectDialog()
{
    delete ui;
}
void ConnectDialog::on_buttonBox_accepted(){

    if(ui->ipEdit->text().size() &&
            ui->portEdit->text().size()){
        host = QHostAddress(ui->ipEdit->text());
        port = ui->portEdit->text().toInt();
        if(host.toIPv4Address()!=0)
            isAccepted= true;
    }
    this->close();
}
