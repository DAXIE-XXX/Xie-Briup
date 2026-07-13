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
    void connectToBroker(const QString &url);
    void publishValue(const QString &topic, const QString &value, int qos = 0, bool retain = true);

signals:
    void publishValueSig(const QString &topic, const QString &value, int qos = 0, bool retain = true);
    void connected();

protected slots:
    void publishValueSlot(const QString &topic, const QString &value, int qos = 0, bool retain = true);

protected:
    static MQTTContext *obj;
    MQTTClientManager *manger;
};

#endif // MQTTCONTEXT_H
