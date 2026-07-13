#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>

class MyTcpSocket : public QTcpSocket
{
    friend class MyTcpServer;
    Q_OBJECT
protected:
    explicit MyTcpSocket(QObject *parent = nullptr);

signals:
    void forwardToClient(int clientType, const QByteArray &data);

    void newClientConnected(int type);

public slots:
    void serverForwardToClientSlot(int clientType, const QByteArray &data);

protected slots:
    void readyReadSlot();

protected:
    void handleData(QByteArray &data);
    void handleFrame(const QByteArray &frame);
    int ct = -1; //默认的身份是-1
};

#endif // MYTCPSOCKET_H
