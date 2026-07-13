#include "mqttclientmanager.h"

#include <QUrl>
#include <QTimer>

MQTTClientManager::MQTTClientManager(QObject *parent)
    : QObject{parent},
    pendingReconnectUrl(QString())
{
    mqttClient = new QMqttClient(this);
    if (!mqttClient) {

    }
    mqttClient->setCleanSession(false);

    // 当客户端成功连接到代理服务器时触发 connected 信号
    connect(mqttClient, &QMqttClient::connected, this, &MQTTClientManager::connected);

    // 当客户端与代理服务器断开连接时触发 disconnected 信号
    connect(mqttClient, &QMqttClient::disconnected, this, &MQTTClientManager::disconnected);
    connect(mqttClient, &QMqttClient::disconnected, this, &MQTTClientManager::disconnectedSlot);

    // 错误处理
    connect(mqttClient, &QMqttClient::errorChanged, this, [this](QMqttClient::ClientError error) {
        qWarning() << "MQTT client error:" << error;
    });
}

void MQTTClientManager::connectToBroker(const QString &url)
{
    // 如果URL为空，使用默认配置
    if (url.isEmpty()) {
        return;
    }

    QUrl brokerUrl(url);
    if (!brokerUrl.isValid()) {
        return;
    }

    QString hostname = brokerUrl.host();
    if (hostname.isEmpty()) {
        return;
    }

    quint16 port = brokerUrl.port(1883);

    // 处理认证信息
    if (!brokerUrl.userName().isEmpty()) {
        mqttClient->setUsername(brokerUrl.userName());
        mqttClient->setClientId(brokerUrl.userName() + "_Server");
    }
    if (!brokerUrl.password().isEmpty()) {
        mqttClient->setPassword(brokerUrl.password());
    }

    if (mqttClient->state() == QMqttClient::Connected) {
        mqttClient->disconnectFromHost();
        pendingReconnectUrl = url;
        return;
    }

    // 设置连接参数并连接
    mqttClient->setHostname(hostname);
    mqttClient->setPort(port);
    mqttClient->connectToHost();
}

bool MQTTClientManager::subscribeTopic(const QString &topic, const int &qos)
{
    if (!mqttClient || mqttClient->state() != QMqttClient::Connected) {
        return false;
    }

    if (m_subscriptions.contains(topic)) {
        if(qos == m_subscriptions.value(topic)->qos()) {
            return true;
        }
        // 取消旧订阅 重新订阅
        unsubscribeTopic(topic);
    }

    auto subscription = mqttClient->subscribe(topic, qos);
    if (!subscription) {
        return false;
    }

    // 存储订阅信息
    m_subscriptions.insert(topic, subscription);

    // 连接订阅特定的信号
    connect(subscription, &QMqttSubscription::messageReceived,
            this, &MQTTClientManager::handleSubscriptionMessageSlot);
    connect(subscription, &QMqttSubscription::stateChanged,
            this, &MQTTClientManager::stateChangedSlot);

    return true;
}

bool MQTTClientManager::unsubscribeTopic(const QString &topic)
{
    if (!mqttClient || mqttClient->state() != QMqttClient::Connected) {
        return false;
    }

    if (!m_subscriptions.contains(topic)) {
        return false;
    }

    // 取出并移除本地存储的订阅对象
    auto subscription = m_subscriptions.take(topic);

    // 主动调用 unsubscribe
    subscription->unsubscribe();

    // 断开信号槽连接
    // disconnect(subscription, nullptr, this, nullptr);

    return true;
}

bool MQTTClientManager::publishValue(const QString &topic, const QString &value, int qos, bool retain)
{
    if (mqttClient->state() != QMqttClient::Connected) {
        return false;
    }

    auto result = mqttClient->publish(topic, value.toUtf8(), qos, retain);
    if (result == -1) {
        return false;
    } else {
        return true;
    }
}

void MQTTClientManager::disconnectedSlot()
{
    if (!pendingReconnectUrl.isEmpty()) {
        QTimer::singleShot(1000, this, [this]() {
            connectToBroker(pendingReconnectUrl);
            pendingReconnectUrl.clear();
        });
    }
}

void MQTTClientManager::handleSubscriptionMessageSlot(const QMqttMessage &msg)
{
    QString topic = msg.topic().name();
    QByteArray payload = msg.payload();
    if(!payload.isEmpty()) {
        readSubscriptionMessageSig(topic, payload);
    }/* else {
        readSubscriptionMessageErrSig(topic);
    }*/
}

void MQTTClientManager::stateChangedSlot(QMqttSubscription::SubscriptionState state)
{
    QMqttSubscription *subscription = dynamic_cast<QMqttSubscription *>(sender());
    QString topic;
    for(auto key:m_subscriptions.keys()) {
        if(m_subscriptions.value(key) == subscription) {
            topic = key;
        }
    }
    if(topic.isEmpty()) {
        return ;
    }
    switch(state) {
    case QMqttSubscription::SubscriptionPending: // 等待订阅
        m_subscriptionLastState.insert(topic, QMqttSubscription::SubscriptionPending);
        break;
    case QMqttSubscription::Subscribed: // 订阅成功
        m_subscriptionLastState.insert(topic, QMqttSubscription::Subscribed);
        emit SubscribeResSig(topic, true);
        break;
    case QMqttSubscription::UnsubscriptionPending: // 等待取消订阅
        m_subscriptionLastState.insert(topic, QMqttSubscription::UnsubscriptionPending);
        break;
    case QMqttSubscription::Unsubscribed: // 取消订阅成功
        m_subscriptionLastState.insert(topic, QMqttSubscription::Unsubscribed);
        emit UnsubscribeResSig(topic, true);
        disconnect(subscription, nullptr, this, nullptr);
        subscription->deleteLater();
        break;
    case QMqttSubscription::Error:
        // 订阅、取消订阅失败
        if(m_subscriptionLastState.value(topic)==QMqttSubscription::SubscriptionPending) emit SubscribeResSig(topic, false);
        else if(m_subscriptionLastState.value(topic)==QMqttSubscription::UnsubscriptionPending) emit UnsubscribeResSig(topic, false);
        m_subscriptionLastState.insert(topic, QMqttSubscription::Error);
        break;
    default:break;
    }
}
