#include "configcontext.h"
#include <QSettings>
#define CONF_FILENAME    "crosssing_conf.ini"
#define DEF_GROUP        "set"
#define SERVER_IP        "server_id"
#define SERVER_PORT      "server_port"
#define CROSSING_ID      "crossing_id"
#define LEFT_ID          "left_light_id"
#define LEFT_SHOW        "left_light_show"
#define LEFT_ROAD        "left_Light_road_id"

#define RIGHT_ID         "right_light_id"
#define RIGHT_SHOW       "right_light_show"
#define RIGHT_ROAD       "right_Light_road_id"

#define UP_ID            "up_light_id"
#define UP_SHOW          "up_light_show"
#define UP_ROAD          "up_Light_road_id"

#define DOWN_ID          "down_light_id"
#define DOWN_SHOW        "down_light_show"
#define DOWN_ROAD        "down_Light_road_id"

#define GREEN_TIME       "green_time"

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

void ConfigContext::getCrossingId(int &id)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(CROSSING_ID))
        set.setValue(CROSSING_ID, 0);
    id = set.value(CROSSING_ID).toInt();

    set.endGroup();
}

void ConfigContext::getLeftLightInfo(int &id, bool &show, int &roadId)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(LEFT_ID))
        set.setValue(LEFT_ID, -1);
    id = set.value(LEFT_ID).toInt();

    if(!set.allKeys().contains(LEFT_SHOW))
        set.setValue(LEFT_SHOW, false);
    show = set.value(LEFT_SHOW).toBool();

    if(!set.allKeys().contains(LEFT_ROAD))
        set.setValue(LEFT_ROAD, -1);
    roadId = set.value(LEFT_ROAD).toInt();

    set.endGroup();
}

void ConfigContext::getRightLightInfo(int &id, bool &show, int &roadId)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(RIGHT_ID))
        set.setValue(RIGHT_ID, -1);
    id = set.value(RIGHT_ID).toInt();

    if(!set.allKeys().contains(RIGHT_SHOW ))
        set.setValue(RIGHT_SHOW , false);
    show = set.value(RIGHT_SHOW ).toBool();

    if(!set.allKeys().contains(RIGHT_ROAD ))
        set.setValue(RIGHT_ROAD , -1);
    roadId = set.value(RIGHT_ROAD ).toInt();

    set.endGroup();
}

void ConfigContext::getUpLightInfo(int &id, bool &show, int &roadId)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(UP_ID))
        set.setValue(UP_ID, -1);
    id = set.value(UP_ID).toInt();

    if(!set.allKeys().contains(UP_SHOW))
        set.setValue(UP_SHOW, false);
    show = set.value(UP_SHOW).toBool();

    if(!set.allKeys().contains(UP_ROAD))
        set.setValue(UP_ROAD, -1);
    roadId = set.value(UP_ROAD).toInt();

    set.endGroup();
}

void ConfigContext::getDownLightInfo(int &id, bool &show, int &roadId)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(DOWN_ID))
        set.setValue(DOWN_ID, -1);
    id = set.value(DOWN_ID).toInt();

    if(!set.allKeys().contains(DOWN_SHOW))
        set.setValue(DOWN_SHOW, false);
    show = set.value(DOWN_SHOW).toBool();

    if(!set.allKeys().contains(DOWN_ROAD))
        set.setValue(DOWN_ROAD, -1);
    roadId = set.value(DOWN_ROAD).toInt();

    set.endGroup();
}

void ConfigContext::getGreenTime(int &time)
{
    QSettings set(CONF_FILENAME, QSettings::IniFormat);
    set.beginGroup(DEF_GROUP);

    if(!set.allKeys().contains(GREEN_TIME))
        set.setValue(GREEN_TIME, 5);
    time = set.value(GREEN_TIME).toInt();

    set.endGroup();
}


