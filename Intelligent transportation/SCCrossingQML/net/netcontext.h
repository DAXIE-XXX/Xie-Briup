#ifndef NETCONTEXT_H
#define NETCONTEXT_H

#include <QObject>
class QTcpSocket;
class NetContext : public QObject
{
    Q_OBJECT

protected:
    explicit NetContext(QObject *parent = nullptr);

public:
    static NetContext *getObject();
    void connectToServer(QString ip, quint16 port);
    void sendType(int type);
    Q_INVOKABLE void sendCrossingMode(int id, int mode);
    void sendModeStatus(int id, int mode);
    void sendLightColorChanged(int id, int roadId, int color);

signals:
    void crossingModeSet(int id, int mode);

protected slots:
    void connectedSlot();
    void disconnectSlot();
    void readyReadSlot();

protected:
    void timerEvent(QTimerEvent *e);
    void handleData(QByteArray &data);
    void handleFrame(const QByteArray &frame);
    static NetContext *obj;
    QString ip;
    quint16 port;
    QTcpSocket *socket;
    int reConId = -1;
};

#endif // NETCONTEXT_H