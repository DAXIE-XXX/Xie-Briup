#ifndef MQTTPUBLISHER_H
#define MQTTPUBLISHER_H

#include <QObject>
#include <QMqttClient>
#include <QThread>

class QSemaphore;

/**
 * @brief MQTT发布单元
 *        每个实例拥有独立的QThread和QMqttClient，用于线程安全的并行发布
 */
class MQTTPublisher : public QObject
{
    Q_OBJECT
    friend class MQTTContext;
public:
    /** @brief 构造并启动MQTT连接（在独立线程上创建QMqttClient）
     *  @param brokerUrl MQTT代理URL (mqtt://user:pass@host:port)
     *  @param clientId  客户端ID（需全局唯一）
     *  @param parent    父对象 */
    explicit MQTTPublisher(const QString &brokerUrl, const QString &clientId, QObject *parent = nullptr);

    /** @brief 析构，停止工作线程并释放QMqttClient */
    ~MQTTPublisher();

    /** @brief 检查MQTT连接是否就绪
     *  @return 已连接返回true */
    bool isConnected();

    /** @brief 阻塞等待连接就绪
     *  @param timeoutMs 超时时间（毫秒） */
    void waitReady(int timeoutMs = 5000);

public slots:
    /** @brief 发布消息到指定主题（通过BlockingQueuedConnection在MQTT线程执行）
     *  @param topic   目标主题
     *  @param value   消息内容
     *  @param qos     服务质量等级 (0/1/2)
     *  @param retain  是否保留消息
     *  @return 是否发布成功 */
    bool publishValue(const QString &topic, const QString &value, int qos = 0, bool retain = true);

signals:
    /** @brief MQTT连接成功信号 */
    void connected();

private:
    QMqttClient *mqttClient;        ///< MQTT客户端（运行在workerThread上）
    QThread *workerThread;           ///< 独立工作线程
    QSemaphore *m_readySemaphore;    ///< 连接就绪信号量
    QString m_brokerUrl;             ///< 代理URL
    QString m_clientId;              ///< 客户端ID
    bool m_connected;                ///< 连接状态
};

#endif // MQTTPUBLISHER_H
