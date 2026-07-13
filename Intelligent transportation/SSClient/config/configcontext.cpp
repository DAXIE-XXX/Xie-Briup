#include "configcontext.h"
#include <QSettings>
#include <QStringList>

#define CONF_FILENAME    "car_conf.ini"
#define DEF_GROUP        "set"
#define SERVER_IP        "server_id"
#define SERVER_PORT      "server_port"
#define CAR_ID           "car_id"
#define CAR_KEY          "car_key"
#define CAR_RUN_LIST     "car_run_list"

// 原有的服务器配置
#define SERVER_CONF_FILENAME    "server_conf.ini"
#define SERVER_DEF_GROUP        "server"
#define LISTEN_PORT      "listen_port"
#define CLIENT_LIMIT     "client_limit"

ConfigContext::ConfigContext(QObject *parent)
    : QObject{parent}
{}

int ConfigContext::getServerListenPort()
{
    QSettings set(SERVER_CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(SERVER_DEF_GROUP);
    if(!set.allKeys().contains(LISTEN_PORT))
        set.setValue(LISTEN_PORT, 10086);
    int port = set.value(LISTEN_PORT).toInt();
    set.endGroup();
    return port;
}

int ConfigContext::getServerMaxClient()
{
    QSettings set(SERVER_CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(SERVER_DEF_GROUP);
    if(!set.allKeys().contains(CLIENT_LIMIT))
        set.setValue(CLIENT_LIMIT, 500);
    int limit = set.value(CLIENT_LIMIT).toInt();
    set.endGroup();
    return limit;
}

// 新增：获取服务器信息
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

// 新增：获取车辆信息
void ConfigContext::getCarInfo(int &id, QString &Key)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(CAR_ID))
        set.setValue(CAR_ID, 0);
    id = set.value(CAR_ID).toInt();

    if(!set.allKeys().contains(CAR_KEY))
        set.setValue(CAR_KEY, "ADMIN123");
    Key = set.value(CAR_KEY).toString();

    set.endGroup();
}

// 新增：获取车辆运行路线
void ConfigContext::getCarRunList(QStringList &list)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(CAR_RUN_LIST)){
        QStringList temp;
        temp << "0" << "6" << "10" << "5" << "13" << "9";
        set.setValue(CAR_RUN_LIST, temp);
    }
    list = set.value(CAR_RUN_LIST).toStringList();

    set.endGroup();
}