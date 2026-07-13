#ifndef MQTTCLIENTMANAGER_H
#define MQTTCLIENTMANAGER_H

#include <QMqttClient>
#include <QObject>
#include <QMqttSubscription>

class MQTTClientManager : public QObject
{
    Q_OBJECT
public:
    explicit MQTTClientManager(QObject *parent = nullptr);
    bool connectToBroker(const QString &url);
    bool subscribeTopic(const QString &topic, const int &qos);
    bool unsubscribeTopic(const QString &topic);
    bool isSubscribed(const QString &topic) {
        return m_subscriptions.contains(topic);
    }
    bool publishValue(const QString &topic, const QString &value, int qos = 0, bool retain = true);

public slots:
    void publishValue2(const QString &topic, const QString &value, int qos = 0, bool retain = true);


signals:
    void connected();
    void disconnected();
    void SubscribeResSig(const QString &topic, const bool &res);
    void UnsubscribeResSig(const QString &topic, const bool &res);
    void readSubscriptionMessageSig(const QString &topic, const QByteArray &value);
    void readSubscriptionMessageErrSig(const QString &topic);

private slots:
    void disconnectedSlot();
    void handleSubscriptionMessageSlot(const QMqttMessage &msg);
    void stateChangedSlot(QMqttSubscription::SubscriptionState state);


private:
    QString pendingReconnectUrl;

    QMqttClient *mqttClient = nullptr;
    QMap<QString, QMqttSubscription*> m_subscriptions;
    QMap<QString, QMqttSubscription::SubscriptionState> m_subscriptionLastState;

};

#endif // MQTTCLIENTMANAGER_H
