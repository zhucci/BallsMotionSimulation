#ifndef SynchronizationMANAGER_H
#define SynchronizationMANAGER_H
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkSession>
#include <QMap>
#include "ball.h"


enum SynchroStatus{Start, Stop, Reset, AddBalls, GetBalls};

struct tcpAddr{
    tcpAddr(QHostAddress h, int p):host(h),port(p){}
    tcpAddr(){}
    tcpAddr(const tcpAddr& ad):host(ad.host),port(ad.port){}
    QHostAddress host;
    int port;
    bool operator==(tcpAddr &addr){
        if(host.toIPv4Address() == (addr.host.toIPv4Address()) && port == addr.port)
            return true;
        return false;
    }
};

class SynchronizationManager :public  QObject
{
    Q_OBJECT
public:

    SynchronizationManager(QObject *parent);

    ~SynchronizationManager();

    int ConnectToPeer(QHostAddress addr, int port, QByteArray &data);
    int SyncData(QByteArray &array);
    int GetServerAddress(QString &addr);
    int GetServerPort(){return tcpServer->serverPort();}
    quint32 GetServerIP(){return tcpServer->serverAddress().toIPv4Address();}

private:
    bool HasSuchClient(tcpAddr newAddr);
private slots:

    void socketError(QAbstractSocket::SocketError err);
    void recieve();
    void acceptPeer();
     int destroySocket();
    void sessionOpened();

signals:

    void ReadData(tcpAddr host, QByteArray byteArray);
    void needSendData(tcpAddr mesSender);
    void ErrorOccure(QString message);
    void serverAddressChanged(QString addr);

private:

      QByteArray *data;
         quint16 blockSize;
             int synchroState;
    QHostAddress ipAddress;
      QTcpServer  *tcpServer;
             int  port;

    QVector <tcpAddr> peers;

};

#endif // SynchronizationMANAGER_H
