#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
class ThreadsHandle;
class MyTcpServer : public QTcpServer
{
    friend class NetContext;
    Q_OBJECT
protected:
    explicit MyTcpServer(QObject *parent = nullptr);

protected slots:
    void socketDisconnectedSlot();

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    int limit = 500;
    int count = 0;
    ThreadsHandle *tsh;
};

#endif // MYTCPSERVER_H














