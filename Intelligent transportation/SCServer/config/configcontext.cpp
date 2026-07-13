#include "configcontext.h"
#include <QSettings>
#define CONF_FILENAME    "server_conf.ini"
#define DEF_GROUP        "server"
#define LISTEN_PORT      "listen_port"
#define CLIENT_LIMIT     "client_limit"

ConfigContext::ConfigContext(QObject *parent)
    : QObject{parent}
{}

int ConfigContext::getServerListenPort()
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);
    if(!set.allKeys().contains(LISTEN_PORT))
        set.setValue(LISTEN_PORT, 10086);
    int port = set.value(LISTEN_PORT).toInt();
    set.endGroup();
    return port;
}

int ConfigContext::getServerMaxClient()
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);
    if(!set.allKeys().contains(CLIENT_LIMIT))
        set.setValue(CLIENT_LIMIT, 500);
    int limit = set.value(CLIENT_LIMIT).toInt();
    set.endGroup();
    return limit;
}
