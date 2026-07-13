#include "systemcontrol.h"
#include "netcontext.h"
#include "logcontext.h"
#include "configcontext.h"

SystemControl::SystemControl(QObject *parent)
    : QObject{parent}
{
    connect(NetContext::getObject(),
            &NetContext::newClientConnected,
            this,
            &SystemControl::newClientConnectedSlot);
    connect(NetContext::getObject(),
            &NetContext::clientDisconnected,
            this,
            &SystemControl::clientDisconnectedSlot);
}

void SystemControl::systemInit()
{
    LogContext::getObject();
    NetContext::getObject();
    LogContext::getObject()->showLog("System init.");
    int limit = ConfigContext::getServerMaxClient();
    NetContext::getObject()->setClientLimit(limit);
    LogContext::getObject()->showLog(
        QString("Server client limit:%1.")
            .arg(limit));
}

void SystemControl::systemStart()
{
    int port = ConfigContext::getServerListenPort();
    bool ok = NetContext::getObject()->startServer(port);
    LogContext::getObject()->showLog(
        QString("System start,listen:%1/%2.")
            .arg(port).arg(ok ? "ok" : "failed"));
}

void SystemControl::newClientConnectedSlot(int type)
{
    LogContext::getObject()->showLog(
        QString("New client:%1.")
            .arg(type));
}

void SystemControl::clientDisconnectedSlot(int type)
{
    LogContext::getObject()->showLog(
        QString("Client disconnected:%1.")
            .arg(type));
}

