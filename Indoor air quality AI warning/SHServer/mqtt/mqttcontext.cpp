#include "mqttcontext.h"
#include "mqttpublisher.h"
#include <QSemaphore>
#include <QUrl>

MQTTContext *MQTTContext::obj = nullptr;

MQTTContext::MQTTContext(QObject *parent)
    : QObject{parent}
{
}

MQTTContext *MQTTContext::getObject()
{
    if(obj == nullptr)
        obj = new MQTTContext;
    return obj;
}

void MQTTContext::setConfig(const QString &url, int poolSize)
{
    if(semaphore != nullptr)
        return;
    this->brokerUrl = url;
    this->poolSize = poolSize > 0 ? poolSize : 10;
}

void MQTTContext::initContext()
{
    if(semaphore != nullptr)
        return;
    if(poolSize <= 0)
        return;
    semaphore = new QSemaphore{poolSize};
    QUrl url(brokerUrl);
    QString user = url.userName();
    for(int i = 0; i < poolSize; i++){
        QString clientId = QString("%1_Pub_%2").arg(user).arg(i + 1);
        MQTTPublisher *pub = new MQTTPublisher(brokerUrl, clientId, this);
        if(i == 0)
            connect(pub, &MQTTPublisher::connected, this, &MQTTContext::connected);
        pub->setProperty("Acquire", false);
        publisherList.append(pub);
    }
}

bool MQTTContext::getState()
{
    if(semaphore == nullptr)
        return false;
    int successCount = 0;
    for(int i = 0; i < publisherList.size(); i++){
        if(publisherList.at(i)->isConnected())
            successCount++;
    }
    return successCount > 0;
}

QString MQTTContext::getStateMsg()
{
    if(semaphore == nullptr)
        return QString("Not init.");
    int successCount = 0, errorCount = 0;
    for(int i = 0; i < publisherList.size(); i++){
        if(publisherList.at(i)->isConnected())
            successCount++;
        else
            errorCount++;
    }
    return QString("MQTT %1@%2 connect %3. limit:%4, successed:%5, failed:%6.")
        .arg(QUrl(brokerUrl).userName())
        .arg(QUrl(brokerUrl).host())
        .arg((successCount > 0 ? "successed" : "failed"))
        .arg(poolSize).arg(successCount).arg(errorCount);
}

MQTTPublisher *MQTTContext::getPublisher()
{
    if(semaphore == nullptr)
        return nullptr;
    semaphore->acquire();
    for(int i = 0; i < publisherList.size(); i++){
        MQTTPublisher *pub = publisherList.at(i);
        if(pub->property("Acquire").toBool() == false){
            pub->setProperty("Acquire", true);
            return pub;
        }
    }
    return nullptr;
}

void MQTTContext::releasePublisher(MQTTPublisher *pub)
{
    if(pub == nullptr)
        return;
    if(pub->property("Acquire").toBool() == false)
        return;
    pub->setProperty("Acquire", false);
    semaphore->release();
}
