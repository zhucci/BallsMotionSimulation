#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QHostAddress>

namespace Ui {
class ConnectDialog;
}

class ConnectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectDialog(QWidget *parent = 0);
    ~ConnectDialog();
    bool IsAccepted(){return isAccepted;}
    std::pair<QHostAddress, int> GetAddres(){
        return std::pair<QHostAddress, int>(host,port);
    }

private slots:
    void on_buttonBox_accepted();

 private:
    bool isAccepted;
    QHostAddress host;
    int port;
    Ui::ConnectDialog *ui;
};

#endif // CONNECTDIALOG_H
