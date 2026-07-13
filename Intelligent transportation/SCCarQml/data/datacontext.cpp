#include "datacontext.h"
#include <QJsonObject>
DataContext *DataContext::obj = nullptr;
double DataContext::getCarEndurance()
{
    return endurance;
}

double DataContext::getCarMileage()
{
    return mileage;
}

DataContext::DataContext(QObject *parent)
    : QObject{parent}
{
    cll.append(new CarLight(17, 112, 35, 36));
    cll.append(new CarLight(216, 112, 35, 36));
    call.append(new CarAlarmLight(17, 155, 35, 15));
    call.append(new CarAlarmLight(216, 155, 35, 15));
}

DataContext *DataContext::getObject()
{
    if(obj == nullptr)
        obj = new DataContext;
    return obj;
}

void DataContext::setCarLightSw(bool sw)
{
    carLightSw = sw;
    emit carLightSwChanged();
}

void DataContext::setCarAlarmSw(bool sw)
{
    carAlarmSw = sw;
    emit carAlarmSwChanged();
}

bool DataContext::getCarLightSw()
{
    return carLightSw;
}

bool DataContext::getCarAlarmSw()
{
    return carAlarmSw;
}

QJsonArray DataContext::getCarLightList()
{
    QJsonArray arr;
    for(int i = 0; i < cll.size(); i++){
        QJsonObject obj;
        obj.insert("x", cll.at(i)->x);
        obj.insert("y", cll.at(i)->y);
        obj.insert("w", cll.at(i)->w);
        obj.insert("h", cll.at(i)->h);
        arr.append(obj);
    }
    return arr;
}

QJsonArray DataContext::getCarAlarmLightList()
{
    QJsonArray arr;
    for(int i = 0; i < call.size(); i++){
        QJsonObject obj;
        obj.insert("x", call.at(i)->x);
        obj.insert("y", call.at(i)->y);
        obj.insert("w", call.at(i)->w);
        obj.insert("h", call.at(i)->h);
        arr.append(obj);
    }
    return arr;
}

void DataContext::setCrossingLight(int roadId, int color)
{
    crossingLightList.insert(roadId, color);
}

int DataContext::getCrossingLight(int roadId)
{
    return crossingLightList.value(roadId);
}

void DataContext::updateCarMileage(double delta)
{
    mileage += delta;
    emit dataChanged();  // 如果需要可以添加这个信号
}

void DataContext::updateCarEndurance(double delta)
{
    endurance += delta;
    if (endurance < 0) endurance = 0;
    emit dataChanged();  // 如果需要可以添加这个信号
}

CarLight::CarLight(int x, int y, int w, int h)
    : x(x), y(y), w(w), h(h)
{}

CarAlarmLight::CarAlarmLight(int x, int y, int w, int h)
    : x(x), y(y), w(w), h(h)
{}

