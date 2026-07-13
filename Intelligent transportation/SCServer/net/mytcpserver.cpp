#include "mytcpserver.h"
#include "mytcpsocket.h"
#include <QThread>

MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer{parent}
{}

void MyTcpServer::socketDisconnectedSlot()
{
    MyTcpSocket *socket = qobject_cast<MyTcpSocket *>(sender());
    count--;
    disconnect(socket, nullptr, nullptr, nullptr);
    disconnect(this, nullptr, socket, nullptr);
    QThread *th = socket->thread();
    int ct = socket->ct;
    socket->deleteLater();
    th->quit();
    if(ct != -1)
        emit clientDisconnected(ct);
}

void MyTcpServer::threadFinishedSlot()
{
    QThread *thread = qobject_cast<QThread *>(sender());
    thread->deleteLater();
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    MyTcpSocket *socket = new MyTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    if(count >= limit){
        // socket->abort();
        // delete socket;
        socket->disconnectFromHost();
        socket->deleteLater();
        return;
    }
    count++;

    connect(socket, &MyTcpSocket::disconnected, this, &MyTcpServer::socketDisconnectedSlot);

    connect(socket, &MyTcpSocket::forwardToClient, this, &MyTcpServer::serverForwardToClient);
    connect(this, &MyTcpServer::serverForwardToClient, socket, &MyTcpSocket::serverForwardToClientSlot);

    connect(socket, &MyTcpSocket::newClientConnected, this, &MyTcpServer::newClientConnected);

    QThread *th = new QThread;
    th->start();
    socket->moveToThread(th);
    connect(th, &QThread::finished, this, &MyTcpServer::threadFinishedSlot);
}
