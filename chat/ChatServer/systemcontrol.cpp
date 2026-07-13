#include "systemcontrol.h"
#include "netcontext.h"
#include "logcontext.h"
#include "configcontext.h"
#include "dbcontext.h"
#include "mqttcontext.h"

SystemControl::SystemControl(QObject *parent)
    : QObject{parent}
{
    connect(MQTTContext::getObject(),
            &MQTTContext::connected,
            this,
            &SystemControl::mqttConnectedSlot);
}

void SystemControl::systemInit(QString path)
{
    ConfigContext::setConfigFilePath(path);
    LogContext::getObject();
    NetContext::getObject();
    MQTTContext::getObject();
    LogContext::getObject()->showLog("System init.");
    LogContext::getObject()->showLog(QString("Config file path:%1.").arg(path));
    int limit = ConfigContext::getServerMaxClient();
    NetContext::getObject()->setClientLimit(limit);
    LogContext::getObject()->showLog(
        QString("Server client limit:%1.")
            .arg(limit));
    LogContext::getObject()->showLog("Start init db connect.");
    limit = ConfigContext::getDBLimit();
    QString ip, un, pd, name;
    quint16 port;
    ConfigContext::getDBConfig(ip, port, un, pd, name);
    DBContext::getObject()->setConfig(
        limit, ip, port, un, pd, name);
    DBContext::getObject()->initContext();
    if(DBContext::getObject()->getState())
        LogContext::getObject()->showLog(
            DBContext::getObject()->getStateMsg());
    else
        LogContext::getObject()->showCritical(
            DBContext::getObject()->getStateMsg());
}

void SystemControl::systemStart()
{
    int port = ConfigContext::getServerListenPort();
    bool ok = NetContext::getObject()->startServer(port);
    if(ok)
        LogContext::getObject()->showLog(
            QString("System start,listen:%1/%2.")
                .arg(port).arg(ok ? "ok" : "failed"));
    else
        LogContext::getObject()->showCritical(
            QString("System start,listen:%1/%2.")
                .arg(port).arg(ok ? "ok" : "failed"));
    LogContext::getObject()->showLog(
        QString("MQTT start connect."));
    mqttCheck = startTimer(5000);
    MQTTContext::getObject()
             ->connectToBroker(
                 "mqtt://briup:123456@127.0.0.1:1883");
}

void SystemControl::mqttConnectedSlot()
{
    LogContext::getObject()->showLog(
        QString("MQTT connected."));
    if(mqttCheck >= 0){
        killTimer(mqttCheck);
        mqttCheck = -1;
    }
}

void SystemControl::timerEvent(QTimerEvent *e)
{
    killTimer(mqttCheck);
    mqttCheck = -1;
    LogContext::getObject()->showCritical(
        QString("MQTT connect timeout."));
}








