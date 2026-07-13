#include "mqttcontext.h"
#include "mqttclientmanager.h"
MQTTContext *MQTTContext::obj = nullptr;
MQTTContext::MQTTContext(QObject *parent)
    : QObject{parent}
    , manger{new MQTTClientManager(this)}
{
    connect(this, &MQTTContext::publishValueSig,
            manger, &MQTTClientManager::publishValue2);
    connect(manger, &MQTTClientManager::readSubscriptionMessageSig,
            this, &MQTTContext::readSubscriptionMessageSig);
    connect(manger, &MQTTClientManager::connected,
            this, &MQTTContext::connected);
}

MQTTContext *MQTTContext::getObject()
{
    if(obj == nullptr)
        obj = new MQTTContext;
    return obj;
}

bool MQTTContext::connectToBroker(const QString &url)
{
    return manger->connectToBroker(url);
}
bool MQTTContext::subscribeTopic(const QString &topic, const int &qos)
{
    return manger->subscribeTopic(topic, qos);
}
bool MQTTContext::unsubscribeTopic(const QString &topic)
{
    return manger->unsubscribeTopic(topic);
}
bool MQTTContext::isSubscribed(const QString &topic)
{
    return manger->isSubscribed(topic);
}
bool MQTTContext::publishValue(const QString &topic, const QString &value, int qos, bool retain)
{
    return manger->publishValue(topic, value, qos, retain);
}




