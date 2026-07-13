#include "systemcontrol.h"
#include "netcontext.h"
#include "logcontext.h"
#include "configcontext.h"
#include "dbcontext.h"

SystemControl::SystemControl(QObject *parent)
    : QObject{parent}
{
}

void SystemControl::systemInit(QString path)
{
    ConfigContext::setConfigFilePath(path);
    LogContext::getObject();
    NetContext::getObject();
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
}
