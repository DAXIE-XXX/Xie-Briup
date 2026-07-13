// netcontext.h
#ifndef NETCONTEXT_H
#define NETCONTEXT_H

#include <QObject>
class QTcpSocket;
class QJsonObject;  // 前向声明

class NetContext : public QObject
{
    Q_OBJECT
protected:
    explicit NetContext(QObject *parent = nullptr);

public:
    static NetContext *getObject();
    void connectToServer(QString ip, quint16 port);
    void sendType(int type);
    void sendCarRunPos(int id, int roadId, int roadPos);

    // 添加这个方法
    void sendStateUpdate(const QJsonObject &data);

signals:
    void carLockSet(int id, bool sw, QString key);
    void carLightSet(int id, bool sw, QString key);
    void carAlarmLightSet(int id, bool sw, QString key);
    void carRunSet(int id, bool sw, QString key);
    void lightColorChanged(int lightId, int roadId, int color);

protected slots:
    void connectedSlot();
    void disconnectedSlot();
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