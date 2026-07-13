#include "configcontext.h"
#include <QSettings>
// #define CONF_FILENAME   "server_conf.ini"
#define DEF_GROUP       "server"
#define LISTEN_PORT     "listen_port"
#define CLIENT_LIMIT    "client_limit"
#define DB_LIMIT        "db_limit"
#define DB_HOST         "db_host"
#define DB_PORT         "db_port"
#define DB_UN           "db_username"
#define DB_PD           "db_password"
#define DB_NAME         "db_name"


QString ConfigContext::CONF_FILENAME = "server_conf.ini";
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

void ConfigContext::setConfigFilePath(QString path)
{
    CONF_FILENAME = path;
}

int ConfigContext::getDBLimit()
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);
    if(!set.allKeys().contains(DB_LIMIT))
        set.setValue(DB_LIMIT, 50);
    int limit = set.value(DB_LIMIT).toInt();
    set.endGroup();
    return limit;
}

void ConfigContext::getDBConfig(QString &ip, quint16 &port, QString &username, QString &password, QString &dbName)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(DB_HOST))
        set.setValue(DB_HOST, "127.0.0.1");
    ip = set.value(DB_HOST).toString();

    if(!set.allKeys().contains(DB_PORT))
        set.setValue(DB_PORT, 13307);
    port = set.value(DB_PORT).toUInt();

    if(!set.allKeys().contains(DB_UN))
        set.setValue(DB_UN, "root");
    username = set.value(DB_UN).toString();

    if(!set.allKeys().contains(DB_PD))
        set.setValue(DB_PD, "123456");
    password = set.value(DB_PD).toString();

    if(!set.allKeys().contains(DB_NAME))
        set.setValue(DB_NAME, "chat");
    dbName = set.value(DB_NAME).toString();

    set.endGroup();
}






