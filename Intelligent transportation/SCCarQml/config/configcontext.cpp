#include "configcontext.h"
#include <QSettings>
#define CONF_FILENAME   "car_conf.ini"
#define DEF_GROUP       "set"
#define SERVER_IP       "server_ip"
#define SERVER_PORT     "server_port"
#define CAR_ID          "car_id"
#define CAR_KEY         "car_key"
#define CAR_RUN_LIST    "car_run_list"

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

void ConfigContext::getCarInfo(int &id, QString &key)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(CAR_ID))
        set.setValue(CAR_ID, 0);
    id = set.value(CAR_ID).toInt();

    if(!set.allKeys().contains(CAR_KEY))
        set.setValue(CAR_KEY, "ADMIN123");
    key = set.value(CAR_KEY).toString();

    set.endGroup();
}

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








