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

    bool operator==(tcpAddr &addr){
        if(host.toIPv4Address() == (addr.host.toIPv4Address()) && port == addr.port)
            return true;
        return false;
    }

    QHostAddress host;
             int port;
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
    int GetServerPort(){return port;}
    quint32 GetServerIP(){return ipAddress.toIPv4Address();}
    bool HasSuchAddress(tcpAddr newAddr);
    void AddNewAddressToRootTable(tcpAddr address){peers.push_back(address);}

private:

private slots:

    void socketError(QAbstractSocket::SocketError err);
    void recieve();
    void acceptPeer();
     int destroySocket();
    void sessionOpened();

signals:

    void ReadData(QByteArray byteArray);
    void needSendData(tcpAddr mesSender);
    void ErrorOccure(QString message);
    void serverAddressChanged(QString addr);

private:

           QTcpServer *tcpServer;
              quint16 blockSize;
         QHostAddress ipAddress;
                  int port;
    QVector <tcpAddr> peers;

};

#endif // SynchronizationMANAGER_H
