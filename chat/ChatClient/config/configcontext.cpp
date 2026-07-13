#include "configcontext.h"
#include <QSettings>
// #define CONF_FILENAME   "crossing_conf.ini"
#define DEF_GROUP       "set"
#define SERVER_IP       "server_ip"
#define SERVER_PORT     "server_port"

ConfigContext *ConfigContext::obj = nullptr;
QString ConfigContext::CONF_FILENAME = "crossing_conf.ini";
ConfigContext::ConfigContext(QObject *parent)
    : QObject{parent}
{}

void ConfigContext::getServerInfo(QString &ip, quint16 &port)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(SERVER_IP))
        set.setValue(SERVER_IP, "127.0.0.1");
    ip = set.value(SERVER_IP).toString();

    if(!set.allKeys().contains(SERVER_PORT))
        set.setValue(SERVER_PORT, 10086);
    port = set.value(SERVER_PORT).toInt();

    set.endGroup();
}


ConfigContext *ConfigContext::getObject()
{
    if(obj == nullptr)
        obj = new ConfigContext;
    return obj;
}

QJsonObject ConfigContext::getConfigPageInfo()
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(SERVER_IP))
        set.setValue(SERVER_IP, "127.0.0.1");
    QString ip = set.value(SERVER_IP).toString();

    if(!set.allKeys().contains(SERVER_PORT))
        set.setValue(SERVER_PORT, 10086);
    int port = set.value(SERVER_PORT).toInt();

    QJsonObject reObj;
    reObj.insert("ip", ip);
    reObj.insert("port", port);
    reObj.insert("path", CONF_FILENAME);
    set.endGroup();
    return reObj;
}

void ConfigContext::setConfigPageInfo(QString ip, int port)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);
    set.setValue(SERVER_IP, ip);
    set.setValue(SERVER_PORT, port);

    set.endGroup();
}

void ConfigContext::setConfigFilePath(QString path)
{
    CONF_FILENAME = path;
}

