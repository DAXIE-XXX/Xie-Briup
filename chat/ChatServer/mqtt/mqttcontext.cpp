#include "mqttcontext.h"
#include "mqttclientmanager.h"
MQTTContext *MQTTContext::obj = nullptr;
MQTTContext::MQTTContext(QObject *parent)
    : QObject{parent}
    , manger{new MQTTClientManager(this)}
{
    connect(this, &MQTTContext::publishValueSig,
            this, &MQTTContext::publishValueSlot);
    connect(manger, &MQTTClientManager::connected,
            this, &MQTTContext::connected);
}

MQTTContext *MQTTContext::getObject()
{
    if(obj == nullptr)
        obj = new MQTTContext;
    return obj;
}

void MQTTContext::connectToBroker(const QString &url)
{
    manger->connectToBroker(url);
}

void MQTTContext::publishValue(const QString &topic, const QString &value, int qos, bool retain)
{
    emit publishValueSig(topic, value, qos, retain);
}

void MQTTContext::publishValueSlot(const QString &topic, const QString &value, int qos, bool retain)
{
    manger->publishValue(topic, value, qos, retain);
}




