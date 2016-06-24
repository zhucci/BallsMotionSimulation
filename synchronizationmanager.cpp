#include "synchronizationmanager.h"
#include <QDataStream>
#include <QNetworkConfiguration>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QSettings>

SynchronizationManager::SynchronizationManager(QObject *parent):
    QObject(parent),tcpServer(0),blockSize(0),port(0),synchroState(1)
{
    sessionOpened();
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(acceptPeer()));
}

QByteArray packData(QByteArray &data){
    QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_0);
        out << (quint16) data.size();
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

    QTcpSocket *newConnect = new QTcpSocket(this);
    //Connect to first peer
    if(!peers.size()){

        tcpAddr a(addr,port);

        peers.push_back(a);
    }

    connect(newConnect,SIGNAL(disconnected()),this,SLOT(destroySocket()));

    //connect(newConnect, SIGNAL(error(QAbstractSocket::SocketError)),
   // this, SLOT(socketError(QAbstractSocket::SocketError)));
    //newConnect->abort();
    newConnect->connectToHost(addr,port);

    QByteArray block = packData(data); //add amount of byte header

    if(!newConnect->waitForConnected(3000))
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

       if (blockSize == 0) {
           if (socket->bytesAvailable() < (int)sizeof(quint16))
               return;
           in >> blockSize;
       }

       if (socket->bytesAvailable() < blockSize)
           return;

        quint16 newPort;
        quint32 ip;

        in>>ip;
        in>>newPort;
        QHostAddress peerAddress(ip);


        tcpAddr senderOfMessage(peerAddress,newPort);

        if(!HasSuchClient(senderOfMessage)){
            peers.push_back(senderOfMessage);
        }

        out<<ip;
        out<<newPort;

        for(int i=0;i<blockSize-3;i+=2){
          quint16 data;
          in >> data;
          out<<data;
       }

       socket->close();
       socket->deleteLater();

       blockSize = 0;

       emit ReadData(senderOfMessage,data);

}
bool SynchronizationManager::HasSuchClient(tcpAddr newAddr){
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
