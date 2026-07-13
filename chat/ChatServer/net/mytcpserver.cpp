#include "mytcpserver.h"
#include "mytcpsocket.h"
#include "threadshandle.h"
#include <QThread>

MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer{parent}
    , tsh{new ThreadsHandle(this)}
{}

void MyTcpServer::socketDisconnectedSlot()
{
    MyTcpSocket *socket = qobject_cast<MyTcpSocket *>(sender());
    count--;
    disconnect(socket, nullptr, nullptr, nullptr);
    disconnect(this, nullptr, socket, nullptr);
    QThread *th = socket->thread();
    socket->deleteLater();
    tsh->releaseTh(th);
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    MyTcpSocket *socket = new MyTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    if(count >= limit){
        socket->disconnectFromHost();
        socket->deleteLater();
        return;
    }
    count++;

    connect(socket, &MyTcpSocket::disconnected,
            this, &MyTcpServer::socketDisconnectedSlot);

    QThread *th = tsh->getTh();
    socket->moveToThread(th);

}









