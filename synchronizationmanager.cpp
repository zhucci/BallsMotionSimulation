#include "synchronizationmanager.h"
#include <QDataStream>
#include <QNetworkConfiguration>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QSettings>

SynchronizationManager::SynchronizationManager(QObject *parent):
    QObject(parent),tcpServer(0), blockSize(0),port(0)
{
    sessionOpened();
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptPeer()));
}

QByteArray packData(QByteArray &data){
    QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        quint16 size = data.size();
        out << (quint16) size;
        block.push_back(data);
    return block;
}

void SynchronizationManager::sessionOpened(){

    tcpServer = new QTcpServer(this);

        if (!tcpServer->listen()) {
            emit ErrorOccure(tcpServer->errorString());
            tcpServer->close();
            delete tcpServer;
            return ;
        }

        QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
        // use the first non-localhost IPv4 address
        for (int i = 0; i < ipAddressesList.size(); ++i) {
            if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
                ipAddress = ipAddressesList[i];
                break;
            }
        }

        // if we did not find one, use IPv4 localhost
        if (ipAddress.isNull())
            ipAddress = QHostAddress(QHostAddress::LocalHost);

    port =tcpServer->serverPort();

    emit serverAddressChanged(ipAddress.toString() + QString(" : %1").arg(port,8));
}

int SynchronizationManager::ConnectToPeer(QHostAddress addr , int port, QByteArray &data){

    if(!addr.toIPv4Address())
        return EXIT_FAILURE;

    QTcpSocket *newConnect = new QTcpSocket(this);

    connect(newConnect,SIGNAL(disconnected()),this,SLOT(destroySocket()));

    //connect(newConnect, SIGNAL(error(QAbstractSocket::SocketError)),
    //   this, SLOT(socketError(QAbstractSocket::SocketError)));
    //    newConnect->abort();
    newConnect->connectToHost(addr,port);

    QByteArray block = packData(data); //add amount of byte header

    if(!newConnect->waitForConnected(2000))
        return EXIT_FAILURE;

        newConnect->write(block, block.size());

    return EXIT_SUCCESS;

}

void SynchronizationManager::socketError(QAbstractSocket::SocketError err){

    QTcpSocket* socket = (QTcpSocket*)sender();

    QString strError =
    "Error: " + (err == QAbstractSocket::HostNotFoundError ?
    "The host was not found." :
    err == QAbstractSocket::RemoteHostClosedError ?
    "The remote host is closed." :
    err == QAbstractSocket::ConnectionRefusedError ?
    "The connection was refused." :
    socket->errorString()
    );
    emit ErrorOccure(strError);
}

int SynchronizationManager::destroySocket(){

    QTcpSocket* socket = (QTcpSocket*)sender();
    socket->close();
    socket->deleteLater();
    return EXIT_SUCCESS;
}

int SynchronizationManager::SyncData(QByteArray &data){

    for(auto addr : peers){
       ConnectToPeer(addr.host, addr.port, data);
    }
    return EXIT_SUCCESS;
}


SynchronizationManager::~SynchronizationManager(){

    tcpServer->close();

}

void SynchronizationManager::acceptPeer(){

    //Connect with new peer and add it to list

    QTcpSocket *peerSock = tcpServer->nextPendingConnection();

    QHostAddress peerName = peerSock->peerAddress();

    QString name = peerName.toString();

    connect(peerSock,SIGNAL(readyRead()),this,SLOT(recieve()));
    connect(peerSock,SIGNAL(disconnected()),this,SLOT(destroySocket()));
    //connect(peerSock, SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(socketError(QAbstractSocket::SocketError)));

}

void SynchronizationManager::recieve(){
    //Get data from peer
    QTcpSocket* socket = (QTcpSocket*)sender(); //signal sender but data reciever

    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QDataStream in(socket);

       in.setVersion(QDataStream::Qt_4_0);

       if (!blockSize) {
           if (socket->bytesAvailable() < (int)sizeof(quint16))
               return;
           in >> blockSize;
       }

       if (socket->bytesAvailable() < blockSize)
           return;

        blockSize = 0;

        int dataAmount = socket->bytesAvailable();

        for(int i=0;i<dataAmount;i++){
          quint8 data;
          in >> data;
          out<<data;
       }

       socket->close();
       socket->deleteLater();

       emit ReadData(data);

}
bool SynchronizationManager::HasSuchAddress(tcpAddr newAddr){
    for(int i=0;i<peers.size();i++){
        if(peers[i]==newAddr)
            return true;
    }
    return false;
}

int SynchronizationManager::GetServerAddress(QString &addr){

    QString str(ipAddress.toString() + QString(" : %1 ").arg(port,7));
    addr = str;
    return EXIT_SUCCESS;
}
