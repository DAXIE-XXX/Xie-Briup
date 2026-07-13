#ifndef MQTTCONTEXT_H
#define MQTTCONTEXT_H

#include <QObject>
class MQTTClientManager;
class MQTTContext : public QObject
{
    Q_OBJECT
protected:
    explicit MQTTContext(QObject *parent = nullptr);

public:
    static MQTTContext *getObject();
    bool connectToBroker(const QString &url);
    bool subscribeTopic(const QString &topic, const int &qos);
    bool unsubscribeTopic(const QString &topic);
    bool isSubscribed(const QString &topic);
    bool publishValue(const QString &topic, const QString &value, int qos = 0, bool retain = true);

signals:
    void connected();
    void publishValueSig(const QString &topic, const QString &value, int qos = 0, bool retain = true);
    void readSubscriptionMessageSig(const QString &topic, const QByteArray &value);

protected:
    static MQTTContext *obj;
    MQTTClientManager *manger;
};

#endif // MQTTCONTEXT_H
