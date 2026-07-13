#include "datacontext.h"
#include <QJsonObject>
#include <QDebug>

DataContext *DataContext::obj = nullptr;

DataContext::DataContext(QObject *parent)
    : QObject{parent}
{
    ll.append(new LightInfo(53, 78, 12, 68, 0));   // 左
    ll.append(new LightInfo(165, 78, 12, 68, 1));  // 右
    ll.append(new LightInfo(80, 48, 68, 12, 2));   // 上
    ll.append(new LightInfo(80, 163, 68, 12, 3));  // 下
}

DataContext *DataContext::getObject()
{
    if(obj == nullptr)
        obj = new DataContext;
    return obj;
}

void DataContext::setLightColor(int hc, int vc)
{
    qDebug() << "setLightColor - horizontal:" << hc << "vertical:" << vc;
    for(int i = 0; i < ll.size(); i++){
        if(ll.at(i)->dir == 0 || ll.at(i)->dir == 1){
            ll.at(i)->color = hc;
        }
        else if(ll.at(i)->dir == 2 || ll.at(i)->dir == 3){
            ll.at(i)->color = vc;
        }
    }
    emit lightColorChanged();
}

void DataContext::setLightHColor(int hc)
{
    qDebug() << "setLightHColor - horizontal:" << hc;
    for(int i = 0; i < ll.size(); i++){
        if(ll.at(i)->dir == 0 || ll.at(i)->dir == 1){
            ll.at(i)->color = hc;
        }
    }
    emit lightColorChanged();
}

void DataContext::setLightVColor(int vc)
{
    qDebug() << "setLightVColor - vertical:" << vc;
    for(int i = 0; i < ll.size(); i++){
        if(ll.at(i)->dir == 2 || ll.at(i)->dir == 3){
            ll.at(i)->color = vc;
        }
    }
    emit lightColorChanged();
}

QJsonArray DataContext::getLightList()
{
    QJsonArray arr;
    for(int i = 0; i < ll.size(); i++){
        QJsonObject obj;
        obj.insert("x", ll.at(i)->x);
        obj.insert("y", ll.at(i)->y);
        obj.insert("w", ll.at(i)->w);
        obj.insert("h", ll.at(i)->h);
        obj.insert("color", ll.at(i)->color);
        obj.insert("dir", ll.at(i)->dir);
        obj.insert("show", ll.at(i)->show);
        obj.insert("roadId", ll.at(i)->roadId);
        obj.insert("id", ll.at(i)->id);
        arr.append(obj);
    }
    return arr;
}

void DataContext::setLeftLightInfo(int id, bool show, int roadId)
{
    for(int i = 0; i < ll.size(); i++){
        if(ll.at(i)->dir == 0){
            ll.at(i)->id = id;
            ll.at(i)->show = show;
            ll.at(i)->roadId = roadId;
        }
    }
}

void DataContext::setRightLightInfo(int id, bool show, int roadId)
{
    for(int i = 0; i < ll.size(); i++){
        if(ll.at(i)->dir == 1){
            ll.at(i)->id = id;
            ll.at(i)->show = show;
            ll.at(i)->roadId = roadId;
        }
    }
}

void DataContext::setUpLightInfo(int id, bool show, int roadId)
{
    for(int i = 0; i < ll.size(); i++){
        if(ll.at(i)->dir == 2){
            ll.at(i)->id = id;
            ll.at(i)->show = show;
            ll.at(i)->roadId = roadId;
        }
    }
}

void DataContext::setDownLightInfo(int id, bool show, int roadId)
{
    for(int i = 0; i < ll.size(); i++){
        if(ll.at(i)->dir == 3){
            ll.at(i)->id = id;
            ll.at(i)->show = show;
            ll.at(i)->roadId = roadId;
        }
    }
}

LightInfo::LightInfo(int x, int y, int w, int h, int dir)
    : x(x), y(y), w(w), h(h), dir(dir)
{
    color = 0;  // 默认绿灯
    show = true; // 默认显示
}