#ifndef NETCONTEXT_H
#define NETCONTEXT_H

#include <QObject>
#include <QHostAddress>
#include <QTimerEvent>

class QTcpSocket;

/**
 * @brief 负责和后端服务器的 TCP 通信，并把传感器上报的数据
 *        按协议组织成 JSON 发送。
 *
 * 采用单例模式，整个程序只会创建一个 NetContext 实例。
 */
class NetContext : public QObject
{
    Q_OBJECT
public:
    /** 获取单例对象 */
    static NetContext *getObject();

    /** 连接到指定的服务器 */
    bool connectToServer(const QString &ip, quint16 port);

    /** 只查询当前是否已连接（不产生网络操作） */
    bool isConnected() const;

    /** 向服务器注册设备（房间号），使服务器能识别网关并建立联动映射 */
    void registerToServer(int roomId);

    /** 启用/禁用自动重连（断线后每5秒尝试一次） */
    void setAutoReconnect(bool enable);

    /** 推送各种传感器数据（协议中对应的 type 编号） */
    void pushCo2      (int    value);   // type = 10001
    void pushTe1      (double value);   // type = 10002
    void pushFire     (bool   value);   // type = 10003
    void pushPM25     (int    value);   // type = 10004
    void pushMethane  (bool   value);   // type = 10005
    void pushLight    (int    value);   // type = 10006
    void pushSmoke    (bool   value);   // type = 10007
    void pushHumidity (double value);   // type = 10008

protected slots:
    void readyReadSlot();

protected:
    /** @brief 数据帧拆分（按{}括号匹配） */
    void handleData();
    /** @brief 单帧分发处理 */
    void handleFrame(const QByteArray &frame);
    /** @brief 定时器：健康检查 + 自动重连 */
    void timerEvent(QTimerEvent *event) override;

private:
    explicit NetContext(QObject *parent = nullptr);
    static NetContext *obj;
    QTcpSocket        *client;
    QByteArray         m_buffer;  // TCP接收持久化缓冲区

    /** 将一个 JSON 对象写入 socket，内部统一调用 */
    void sendJson(const QJsonObject &obj);

    // 自动重连相关
    bool        m_autoReconnect = false;
    QString     m_lastIp;
    quint16     m_lastPort = 0;
    int         m_reconnectTimerId = 0;
    int         m_retryCount = 0;
    static inline const int MAX_RETRY_DELAY_MS = 30000;  // 最大重试间隔30秒
    static inline const int BASE_RETRY_DELAY_MS = 5000;   // 基础重试间隔5秒
    void startReconnectTimer();
    void stopReconnectTimer();
    void tryReconnect();
};

#endif // NETCONTEXT_H