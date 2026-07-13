#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include <QJsonObject>
#include <QDateTime>

class MyTcpSocket : public QTcpSocket
{
    friend class MyTcpServer;
    Q_OBJECT

signals:
    void forwardToGateway(const QJsonObject &obj);
    void sensorDataReceived(const QJsonObject &data);
    // ===== 新增：设备注册信号 =====
    void deviceRegistered(MyTcpSocket *socket, int roomId, bool isGateway);

public:
    /** @brief 设置所属服务器引用（因 moveToThread 后 parent() 为 null） */
    void setServer(class MyTcpServer *server) { m_server = server; }

public slots:
    void forwardToGatewaySlot(const QJsonObject &obj);

protected:
    explicit MyTcpSocket(QObject *parent = nullptr);

protected slots:
    void readyReadSlot();

protected:
    void handleData();
    void handleFrame(const QByteArray &frame);
    void handleRegister(const QJsonObject &obj);

    // ===== 新增：联动检查 =====
    void checkAndTrigger(const QString &type, double value);

    class MyTcpServer *m_server = nullptr;
    bool gateway = false;
    int roomId = -1;
    QString deviceId;
    QByteArray m_buffer;                             // TCP接收持久化缓冲区
    QMap<QString, QDateTime> lastTriggerTimes;       // 每种传感器独立的冷却时间
    void handleSensorRequest(int type);
};

#endif // MYTCPSOCKET_H