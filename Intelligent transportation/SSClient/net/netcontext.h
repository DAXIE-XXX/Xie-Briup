#ifndef NETCONTEXT_H
#define NETCONTEXT_H

#include <QObject>
#include <QTcpSocket>

class NetContext : public QObject
{
    Q_OBJECT

protected:
    explicit NetContext(QObject *parent = nullptr);

public:
    static NetContext *getObject();
    void connectToServer(QString ip, quint16 port);
    void sendType(int type);

    // 获取当前连接信息
    Q_INVOKABLE QString getCurrentIp() const { return ip; }
    Q_INVOKABLE quint16 getCurrentPort() const { return port; }
    Q_INVOKABLE bool isConnected() const {
        return socket->state() == QTcpSocket::ConnectedState;
    }

    Q_INVOKABLE void sendCrossingMode(int id, int mode);
    Q_INVOKABLE void sendCarLightSet(int id, bool sw);
    Q_INVOKABLE void sendCarAlarmSet(int id, bool sw);
    Q_INVOKABLE void sendCarLockSet(int id, bool sw);
    Q_INVOKABLE void sendCarRunSet(int id, bool sw);

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
    QTcpSocket *socket;  // 现在可以正确识别 QTcpSocket 类型
    int reConId = -1;

signals:
    void carLightSwChanged(int id, bool sw);
    void carAlarmSwChanged(int id, bool sw);
    void carLockSwChanged(int id, bool sw);
    void carRunSwChanged(int id, bool sw);
    void carRunPosChanged(int id, int roadId, int roadPos);
    void carStartRequested(int id);
    void modeStatusReceived(int id, int mode);
    void lightColorChanged(int id, int roadId, int color);
    void connectionStatusChanged(bool connected);
};

#endif // NETCONTEXT_H