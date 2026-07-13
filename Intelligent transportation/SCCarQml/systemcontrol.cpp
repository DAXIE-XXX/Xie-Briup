#include "systemcontrol.h"
#include "netcontext.h"
#include "datacontext.h"
#include "configcontext.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>

SystemControl *SystemControl::obj = nullptr;

SystemControl::SystemControl(QObject *parent)
    : QObject{parent}
{
    startTimer(100);  // 改为100ms，更流畅
    connect(NetContext::getObject(), &NetContext::carLockSet,
            this, &SystemControl::carLockSetSlot);
    connect(NetContext::getObject(), &NetContext::carLightSet,
            this, &SystemControl::carLightSetSlot);
    connect(NetContext::getObject(), &NetContext::carAlarmLightSet,
            this, &SystemControl::carAlarmLightSetSlot);
    connect(NetContext::getObject(), &NetContext::carRunSet,
            this, &SystemControl::carRunSetSlot);
    connect(NetContext::getObject(), &NetContext::lightColorChanged,
            this, &SystemControl::onLightColorChanged);
}

SystemControl *SystemControl::getObject()
{
    if(obj == nullptr)
        obj = new SystemControl;
    return obj;
}

void SystemControl::systemInit()
{
    NetContext::getObject();
    DataContext::getObject();
    ConfigContext::getCarInfo(carId, carKey);
    ConfigContext::getCarRunList(runList);

    // 初始化红绿灯状态为绿灯
    for(int i = 0; i < 14; i++) {
        lightStates[i] = 0;
    }
}

void SystemControl::systemStart()
{
    QString ip;
    quint16 port;
    ConfigContext::getServerInfo(ip, port);
    NetContext::getObject()->connectToServer(ip, port);
}

int SystemControl::getCarId()
{
    return carId;
}

void SystemControl::setCarLock(bool sw)
{
    lockState = sw;
    if(sw == true)
        runState = false;
    emit lockStateChanged(sw);
    // 发送状态给客户端
    QJsonObject data;
    data.insert("type", 5004);
    data.insert("id", carId);
    data.insert("sw", sw);
    NetContext::getObject()->sendStateUpdate(data);
}

void SystemControl::setCarRun(bool sw)
{
    if(lockState == true)
        return;
    runState = sw;
    emit runStateChanged(sw);
    // 发送状态给客户端
    QJsonObject data;
    data.insert("type", 5001);
    data.insert("id", carId);
    data.insert("sw", sw);
    NetContext::getObject()->sendStateUpdate(data);
}

int SystemControl::getLightColor(int roadId)
{
    return lightStates.value(roadId, 0);
}

// 检查是否被红灯阻挡
bool SystemControl::isBlockedByRedLight(int currentRoadId, int currentRoadPos)
{
    // 只有接近路口（95%以后）才需要检查红灯
    if(currentRoadPos < 95) {
        return false;
    }

    // 根据当前道路获取对应的红绿灯道路ID
    int lightRoadId = -1;

    // 道路映射（根据你的实际道路配置）
    switch(currentRoadId) {
    case 0: lightRoadId = 0; break;
    case 1: lightRoadId = 1; break;
    case 2: lightRoadId = 2; break;
    case 3: lightRoadId = 3; break;
    case 4: lightRoadId = 4; break;
    case 5: lightRoadId = 5; break;
    case 6: lightRoadId = 6; break;
    case 7: lightRoadId = 7; break;
    case 8: lightRoadId = 8; break;
    case 9: lightRoadId = 9; break;
    case 10: lightRoadId = 10; break;
    case 11: lightRoadId = 11; break;
    case 12: lightRoadId = 12; break;
    case 13: lightRoadId = 13; break;
    default: return false;
    }

    int color = lightStates.value(lightRoadId, 0);

    // 红灯（颜色为2）时停车
    if(color == 2) {
        qDebug() << "Car" << carId << "stopped by red light - road:"
                 << currentRoadId << "pos:" << currentRoadPos;
        return true;
    }

    return false;
}

void SystemControl::timerEvent(QTimerEvent *e)
{
    if(runState != true || lockState == true)
        return;
    if(runList.isEmpty())
        return;

    int currentRoadId = runList.at(runRoadIndex).toInt();

    // 检查是否被红灯阻挡
    if(isBlockedByRedLight(currentRoadId, runRoadPos)) {
        return;
    }

    // 检查续航是否耗尽
    if(DataContext::getObject()->getCarEndurance() <= 0) {
        runState = false;
        emit runStateChanged(false);
        qDebug() << "Car" << carId << "ran out of endurance!";
        return;
    }

    if(currentRoadId < 6)
        runRoadPos += 5;
    else
        runRoadPos += 10;

    // 更新里程和续航
    DataContext::getObject()->updateCarMileage(0.1);
    DataContext::getObject()->updateCarEndurance(-0.1);

    if(runRoadPos > 100){
        runRoadIndex++;
        if(runRoadIndex >= runList.length())
            runRoadIndex = 0;
        runRoadPos = 0;
    }

    // 发送位置数据
    NetContext::getObject()->sendCarRunPos(
        carId,
        runList.at(runRoadIndex).toInt(),
        runRoadPos);

    // **重要：** 同时发送续航和里程数据
    QJsonObject enduranceData;
    enduranceData.insert("type", 2009);
    enduranceData.insert("id", carId);
    enduranceData.insert("endurance", DataContext::getObject()->getCarEndurance());
    enduranceData.insert("mileage", DataContext::getObject()->getCarMileage());
    NetContext::getObject()->sendStateUpdate(enduranceData);

    qDebug() << "Car" << carId << "endurance:" << DataContext::getObject()->getCarEndurance()
             << "mileage:" << DataContext::getObject()->getCarMileage();
}

void SystemControl::carLockSetSlot(int id, bool sw, QString key)
{
    if(id != carId || key != carKey)
        return;
    setCarLock(sw);
}

void SystemControl::carLightSetSlot(int id, bool sw, QString key)
{
    if(id != carId || key != carKey)
        return;
    DataContext::getObject()->setCarLightSw(sw);
    // 发送状态给客户端
    QJsonObject data;
    data.insert("type", 5002);
    data.insert("id", carId);
    data.insert("sw", sw);
    NetContext::getObject()->sendStateUpdate(data);
}

void SystemControl::carAlarmLightSetSlot(int id, bool sw, QString key)
{
    if(id != carId || key != carKey)
        return;
    DataContext::getObject()->setCarAlarmSw(sw);
}

void SystemControl::carRunSetSlot(int id, bool sw, QString key)
{
    if(id != carId || key != carKey)
        return;
    setCarRun(sw);
}

void SystemControl::onLightColorChanged(int lightId, int roadId, int color)
{
    qDebug() << "Car" << carId << "light changed - lightId:" << lightId
             << "roadId:" << roadId << "color:" << color;

    // 存储红绿灯状态
    lightStates[roadId] = color;

    if(color == 0) {
        qDebug() << "Car" << carId << "light turned GREEN at road:" << roadId;
    }
}