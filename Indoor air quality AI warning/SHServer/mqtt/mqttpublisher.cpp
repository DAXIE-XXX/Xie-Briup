#include "mqttpublisher.h"
#include <QUrl>
#include <QSemaphore>

MQTTPublisher::MQTTPublisher(const QString &brokerUrl, const QString &clientId, QObject *parent)
    : QObject{parent}
    , mqttClient(nullptr)
    , m_connected(false)
{
    m_brokerUrl = brokerUrl;
    m_clientId = clientId;

    mqttClient = new QMqttClient;
    mqttClient->setCleanSession(false);
    mqttClient->setClientId(m_clientId);
    QUrl url(m_brokerUrl);
    mqttClient->setHostname(url.host());
    mqttClient->setPort(url.port(1883));
    if(!url.userName().isEmpty())
        mqttClient->setUsername(url.userName());
    if(!url.password().isEmpty())
        mqttClient->setPassword(url.password());

    connect(mqttClient, &QMqttClient::connected, this, [this]() {
        m_connected = true;
        emit connected();
    });

    mqttClient->connectToHost();
}

MQTTPublisher::~MQTTPublisher()
{
    if(mqttClient)
        mqttClient->deleteLater();
}

bool MQTTPublisher::isConnected()
{
    return m_connected;
}

bool MQTTPublisher::publishValue(const QString &topic, const QString &value, int qos, bool retain)
{
    if(!m_connected || mqttClient == nullptr)
        return false;
    bool ok = false;
    QMetaObject::invokeMethod(mqttClient, [&topic, &value, qos, retain, &ok, this]() {
        if(mqttClient == nullptr || mqttClient->state() != QMqttClient::Connected)
            return;
        ok = mqttClient->publish(topic, value.toUtf8(), qos, retain) != -1;
    }, Qt::BlockingQueuedConnection);
    return ok;
}
