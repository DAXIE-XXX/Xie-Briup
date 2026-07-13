#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpserver>

class MyTcpServer : public QTcpServer
{
    friend class NetContext;
    Q_OBJECT
protected:
    explicit MyTcpServer(QObject *parent = nullptr);

signals:
    void serverForwardToClient(int clientType, const QByteArray &frame);

    void newClientConnected(int type);
    void clientDisconnected(int type);

protected slots:
    void socketDisconnectedSlot();
    void threadFinishedSlot();

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    int limit = 500;
    int count = 0;
};

#endif // MYTCPSERVER_H
