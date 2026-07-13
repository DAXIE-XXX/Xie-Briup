#ifndef MQTTCONTEXT_H
#define MQTTCONTEXT_H

#include <QObject>
#include <QVector>
class QSemaphore;
class MQTTPublisher;

/**
 * @brief 服务端MQTT连接池管理类（单例，仿DBContext模式）
 *        维护多个MQTTPublisher实例，通过信号量控制并发访问
 */
class MQTTContext : public QObject
{
    Q_OBJECT
protected:
    explicit MQTTContext(QObject *parent = nullptr);

public:
    /** @brief 获取单例实例 */
    static MQTTContext *getObject();

    /** @brief 配置连接池参数（仅可调用一次）
     *  @param url      MQTT代理URL (mqtt://user:pass@host:port)
     *  @param poolSize 连接池大小 */
    void setConfig(const QString &url, int poolSize);

    /** @brief 初始化连接池，创建MQTTPublisher实例 */
    void initContext();

    /** @brief 检查连接池状态（是否所有连接都成功） */
    bool getState();

    /** @brief 获取连接池状态描述 */
    QString getStateMsg();

    /** @brief 从池中获取一个可用的Publisher（阻塞等待）
     *  @return 可用的MQTTPublisher指针，失败返回nullptr */
    MQTTPublisher *getPublisher();

    /** @brief 归还Publisher到池中
     *  @param pub 之前获取的MQTTPublisher指针 */
    void releasePublisher(MQTTPublisher *pub);

signals:
    /** @brief 首个Publisher连接成功信号 */
    void connected();

protected:
    static MQTTContext *obj;
    QSemaphore *semaphore = nullptr;
    QVector<MQTTPublisher *> publisherList;
    int poolSize = 0;
    QString brokerUrl;
};

#endif // MQTTCONTEXT_H
