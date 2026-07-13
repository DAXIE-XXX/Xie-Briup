#include "datacontext.h"
#include "netcontext.h"
#include <QJsonObject>
#include <QTimer>
#include <QDebug>

DataContext::DataContext(QObject *parent)
    : QObject{parent}
{
    // 初始化道路信息 (14条道路)
    rl.append(new RoadInfo(96, 27, 648, 3, 0));   // 0
    rl.append(new RoadInfo(96, 59, 648, 3, 1));   // 1
    rl.append(new RoadInfo(96, 347, 648, 3, 0));  // 2
    rl.append(new RoadInfo(96, 379, 648, 3, 1));  // 3
    rl.append(new RoadInfo(96, 671, 648, 3, 0));  // 4
    rl.append(new RoadInfo(96, 703, 648, 3, 1));  // 5
    rl.append(new RoadInfo(27, 96, 3, 220, 3));   // 6
    rl.append(new RoadInfo(59, 96, 3, 220, 2));   // 7
    rl.append(new RoadInfo(783, 96, 3, 220, 3));  // 8
    rl.append(new RoadInfo(815, 96, 3, 220, 2));  // 9
    rl.append(new RoadInfo(27, 420, 3, 220, 3));  // 10
    rl.append(new RoadInfo(59, 420, 3, 220, 2));  // 11
    rl.append(new RoadInfo(783, 420, 3, 220, 3)); // 12
    rl.append(new RoadInfo(815, 420, 3, 220, 2)); // 13

    // 初始化红绿灯信息 (6个红绿灯)
    ll.append(new LightInfo(6));   // ll[0] = 道路6 -> 路口0
    ll.append(new LightInfo(2));   // ll[1] = 道路2 -> 路口1
    ll.append(new LightInfo(11));  // ll[2] = 道路11 -> 路口2
    ll.append(new LightInfo(8));   // ll[3] = 道路8 -> 路口3
    ll.append(new LightInfo(13));  // ll[4] = 道路13 -> 路口4
    ll.append(new LightInfo(3));   // ll[5] = 道路3 -> 路口5

    // 初始化车辆信息 (3辆车)
    cl.append(new CarInfo());
    cl.append(new CarInfo());
    cl.append(new CarInfo());

    // 获取网络上下文并连接信号
    nc = NetContext::getObject();
    if (nc) {
        connect(nc, &NetContext::carLightSwChanged,
                this, &DataContext::onCarLightSwChanged);
        connect(nc, &NetContext::carAlarmSwChanged,
                this, &DataContext::onCarAlarmSwChanged);
        connect(nc, &NetContext::carRunSwChanged,
                this, &DataContext::onCarRunSwChanged);
        connect(nc, &NetContext::carLockSwChanged,
                this, &DataContext::onCarLockSwChanged);
        connect(nc, &NetContext::lightColorChanged,
                this, &DataContext::onLightColorChanged);
    }

    // 初始化车辆移动状态
    carMoving = QVector<bool>(3, false);

    // 创建移动定时器
    movementTimer = new QTimer(this);
    connect(movementTimer, &QTimer::timeout, this, &DataContext::updateCarPositions);
    movementTimer->start(50);
}

/* ---------------------- 里程/续航 ---------------------- */
double DataContext::getCarEndurance(int id)
{
    if (id < 0 || id >= cl.size()) return 0.0;
    return cl.at(id)->endurance;
}

double DataContext::getCarMileage(int id)
{
    if (id < 0 || id >= cl.size()) return 0.0;
    return cl.at(id)->mileage;
}

/* ---------------------- 本地启动/停止 ---------------------- */
void DataContext::startCarLocally(int id)
{
    if (id < 0 || id >= cl.size()) return;
    if (!cl[id]->isRunning) {
        cl[id]->isRunning = true;
        carMoving[id] = true;
        // 同步给服务器（保持后端一致）
        if (nc) nc->sendCarRunSet(id, true);
        emit dataChanged();
    }
}

void DataContext::stopCarLocally(int id)
{
    if (id < 0 || id >= cl.size()) return;
    if (cl[id]->isRunning) {
        cl[id]->isRunning = false;
        carMoving[id] = false;
        if (nc) nc->sendCarRunSet(id, false);
        emit dataChanged();
    }
}

DataContext *DataContext::obj = nullptr;

DataContext *DataContext::getObject()
{
    if(obj == nullptr)
        obj = new DataContext;
    return obj;
}

QJsonArray DataContext::getRoadList()
{
    QJsonArray arr;
    for(int i = 0; i < rl.size(); i++){
        QJsonObject obj;
        obj.insert("x", rl.at(i)->x);
        obj.insert("y", rl.at(i)->y);
        obj.insert("w", rl.at(i)->w);
        obj.insert("h", rl.at(i)->h);
        obj.insert("color", rl.at(i)->color);
        obj.insert("dir", rl.at(i)->dir);
        arr.append(obj);
    }
    return arr;
}

QJsonArray DataContext::getLightList()
{
    QJsonArray arr;
    for(int i = 0; i < ll.size(); i++){
        QJsonObject obj;
        int id = ll.at(i)->roadId;
        obj.insert("roadId", id);
        obj.insert("color", ll.at(i)->color);
        obj.insert("roadX", rl.at(id)->x);
        obj.insert("roadY", rl.at(id)->y);
        obj.insert("roadW", rl.at(id)->w);
        obj.insert("roadH", rl.at(id)->h);
        obj.insert("roadDir", rl.at(id)->dir);
        arr.append(obj);
    }
    return arr;
}

QJsonArray DataContext::getCarList()
{
    QJsonArray arr;
    for(int i = 0; i < cl.size(); i++){
        QJsonObject obj;
        obj.insert("road_id", cl.at(i)->roadId);
        obj.insert("road_pos", cl.at(i)->roadPos);
        obj.insert("is_running", cl.at(i)->isRunning);
        obj.insert("is_light_on", cl.at(i)->isLightOn);
        obj.insert("is_alarm_on", cl.at(i)->isAlarmOn);
        obj.insert("is_locked", cl.at(i)->isLocked);
        obj.insert("car_id", i);
        arr.append(obj);
    }
    return arr;
}

void DataContext::setLightColor(int id, int color)
{
    if(id >= ll.size())
        return;

    // 颜色值: 0-green, 1-yellow, 2-red, -1-off
    int normalizedColor = color;

    qDebug() << "setLightColor - lightId:" << id << "color:" << normalizedColor;

    if(ll.at(id)->color != normalizedColor) {
        ll.at(id)->color = normalizedColor;

        // 如果变成绿灯，唤醒等待的车辆
        if(normalizedColor == 0) {
            qDebug() << "Light" << id << "turned GREEN, checking waiting cars...";
            for(int i = 0; i < cl.size(); i++) {
                if(cl[i]->isWaitingAtRedLight) {
                    qDebug() << "Car" << i << "was waiting at red light, resuming";
                    cl[i]->isWaitingAtRedLight = false;
                    cl[i]->waitingLightId = -1;
                }
            }
        }

        emit dataChanged();
    }
}

void DataContext::onLightColorChanged(int id, int roadId, int color)
{
    qDebug() << "onLightColorChanged - id:" << id << "roadId:" << roadId << "color:" << color;

    // 直接调用 setLightColor 处理
    setLightColor(id, color);
}

int DataContext::getLightIdAtCarPosition(int carId)
{
    if(carId < 0 || carId >= cl.size()) return -1;
    CarInfo *car = cl[carId];
    if(car->roadId < 0 || car->roadId >= rl.size()) return -1;

    int roadId = car->roadId;
    int roadPos = car->roadPos;

    // 只有接近路口时才返回红绿灯ID
    if (roadPos < 95) return -1;

    qDebug() << "Car" << carId << "at intersection - roadId:" << roadId << "pos:" << roadPos;

    // 道路到红绿灯索引的映射
    switch(roadId) {
    case 6:
    case 10:
        qDebug() << "Mapping road" << roadId << "to light 0";
        return 0;
    case 2:
    case 1:
        qDebug() << "Mapping road" << roadId << "to light 1";
        return 1;
    case 11:
    case 9:
        qDebug() << "Mapping road" << roadId << "to light 2";
        return 2;
    case 8:
    case 7:
        qDebug() << "Mapping road" << roadId << "to light 3";
        return 3;
    case 13:
    case 12:
        qDebug() << "Mapping road" << roadId << "to light 4";
        return 4;
    case 3:
    case 4:
        qDebug() << "Mapping road" << roadId << "to light 5";
        return 5;
    default:
        qDebug() << "Unknown roadId:" << roadId << "no light mapping";
        return -1;
    }
}

bool DataContext::isCarBlockedByRedLight(int carId)
{
    if(carId < 0 || carId >= cl.size()) return false;

    CarInfo *car = cl[carId];
    if(car->roadId < 0 || car->roadId >= rl.size()) return false;

    int roadPos = car->roadPos;
    const int STOP_DISTANCE_THRESHOLD = 95;

    if (roadPos < STOP_DISTANCE_THRESHOLD) {
        if (car->isWaitingAtRedLight) {
            qDebug() << "Car" << carId << "moved away from intersection, clearing wait state";
            car->isWaitingAtRedLight = false;
            car->waitingLightId = -1;
        }
        return false;
    }

    int lightId = getLightIdAtCarPosition(carId);

    if(lightId == -1) {
        qDebug() << "Car" << carId << "NO LIGHT MAPPING for roadId:" << car->roadId;
        return false;
    }

    if(lightId >= ll.size()) {
        qDebug() << "Car" << carId << "lightId:" << lightId << "out of range";
        return false;
    }

    int lightColor = ll[lightId]->color;
    qDebug() << "Car" << carId << "lightId:" << lightId << "color:" << lightColor
             << "(0=green,1=yellow,2=red)";

    // 红灯（颜色值为2）时停车
    if(lightColor == 2) {
        qDebug() << ">>> Car" << carId << "STOPPED by RED LIGHT! <<<";
        car->isWaitingAtRedLight = true;
        car->waitingLightId = ll[lightId]->roadId;
        return true;
    }

    if(car->isWaitingAtRedLight) {
        qDebug() << "Car" << carId << "resumed moving (light not red)";
        car->isWaitingAtRedLight = false;
        car->waitingLightId = -1;
    }

    return false;
}

bool DataContext::getCarRunning(int id)
{
    if (id < 0 || id >= cl.size()) return false;
    return cl.at(id)->isRunning;
}

bool DataContext::getCarLight(int id)
{
    if (id < 0 || id >= cl.size()) return false;
    return cl.at(id)->isLightOn;
}

bool DataContext::getCarAlarm(int id)
{
    if (id < 0 || id >= cl.size()) return false;
    return cl.at(id)->isAlarmOn;
}

bool DataContext::getCarLock(int id)
{
    if (id < 0 || id >= cl.size()) {
        qDebug() << "getCarLight: invalid id" << id;
        return false;
    }
    bool result = cl.at(id)->isLightOn;
    qDebug() << "getCarLight(" << id << ") = " << result;
    return result;
}

void DataContext::setCarRunning(int id, bool running)
{
    if (id < 0 || id >= cl.size()) return;
    if (cl.at(id)->isRunning != running) {
        cl.at(id)->isRunning = running;
        if (nc) {
            nc->sendCarRunSet(id, running);
        }

        if (id < carMoving.size()) {
            carMoving[id] = running;
        }

        emit dataChanged();
    }
}

void DataContext::setCarLight(int id, bool light)
{
    if (id < 0 || id >= cl.size()) return;
    if (cl.at(id)->isLightOn != light) {
        cl.at(id)->isLightOn = light;
        if (nc) {
            nc->sendCarLightSet(id, light);
        }
        emit dataChanged();
    }
}

void DataContext::setCarAlarm(int id, bool alarm)
{
    if (id < 0 || id >= cl.size()) return;
    if (cl.at(id)->isAlarmOn != alarm) {
        cl.at(id)->isAlarmOn = alarm;
        if (nc) {
            nc->sendCarAlarmSet(id, alarm);
        }
        emit dataChanged();
    }
}

void DataContext::setCarLock(int id, bool locked)
{
    if (id < 0 || id >= cl.size()) return;
    if (cl.at(id)->isLocked != locked) {
        cl.at(id)->isLocked = locked;
        if (nc) {
            nc->sendCarLockSet(id, locked);
        }
        emit dataChanged();
    }
}

void DataContext::pageSetCarAlarm(int carId, bool sw)
{
    setCarAlarm(carId, sw);
}

void DataContext::pageSetCarLight(int carId, bool sw)
{
    setCarLight(carId, sw);
}

void DataContext::pageSetCarLock(int carId, bool sw)
{
    setCarLock(carId, sw);
}

void DataContext::pageSetCarRun(int carId, bool sw)
{
    setCarRunning(carId, sw);
}

int DataContext::getCarPosX(int carId)
{
    if (carId < 0 || carId >= cl.size()) return 0;
    CarInfo *car = cl[carId];
    if (car->roadId < 0 || car->roadId >= rl.size()) return 0;
    RoadInfo *road = rl[car->roadId];

    switch (road->dir) {
    case 0:
    case 1:
        return road->x + (car->roadPos / 100.0) * road->w;
    default:
        return road->x + road->w / 2;
    }
}

int DataContext::getCarPosY(int carId)
{
    if (carId < 0 || carId >= cl.size()) return 0;
    CarInfo *car = cl[carId];
    if (car->roadId < 0 || car->roadId >= rl.size()) return 0;
    RoadInfo *road = rl[car->roadId];

    switch (road->dir) {
    case 2:
    case 3:
        return road->y + (car->roadPos / 100.0) * road->h;
    default:
        return road->y + road->h / 2;
    }
}

void DataContext::startCarMovement(int carId)
{
    if (carId >= 0 && carId < carMoving.size()) {
        carMoving[carId] = true;
        if (carId < cl.size()) {
            cl[carId]->isRunning = true;
        }
        emit dataChanged();
    }
}

void DataContext::stopCarMovement(int carId)
{
    if (carId >= 0 && carId < carMoving.size()) {
        carMoving[carId] = false;
        if (carId < cl.size()) {
            cl[carId]->isRunning = false;
        }
        emit dataChanged();
    }
}

QString DataContext::getCarWarning(int id)
{
    if (id < 0 || id >= cl.size()) return "--";

    CarInfo *car = cl[id];
    QStringList warnings;

    // 检查续航不足
    if (car->endurance < 20.0) {
        warnings.append("续航不足");
    }

    // 检查其他警告条件
    if (car->isLocked && car->isRunning) {
        warnings.append("锁定中启动");
    }

    return warnings.isEmpty() ? "--" : warnings.join(", ");
}

void DataContext::updateCarPositions()
{
    bool changed = false;
    for (int i = 0; i < cl.size(); i++) {
        if (!carMoving[i] || !cl[i]->isRunning) continue;

        if (isCarBlockedByRedLight(i)) continue;

        if (cl[i]->roadId >= 0 && cl[i]->roadId < rl.size()) {
            cl[i]->roadPos += 1;
            if (cl[i]->roadPos >= 100) cl[i]->roadPos = 0;

            // **里程/续航递增递减**
            cl[i]->mileage   += 0.1;   // 0.1 km 每次移动
            cl[i]->endurance -= 0.1;
            if (cl[i]->endurance < 0) {
                cl[i]->endurance = 0;
                carMoving[i] = false;
                cl[i]->isRunning = false;
            }

            changed = true;
        }
    }
    if (changed) emit dataChanged();
}

void DataContext::autoSetRoadColor(int roadId)
{
    if(roadId < 0 || roadId >= rl.length())
        return;
    int count = 0;
    for(int i = 0; i < cl.length(); i++)
        if(cl.at(i)->roadId == roadId)
            count++;
    rl.at(roadId)->color = count == 0 ? 0 : count == 1 ? 1: 2;
}

void DataContext::onCarLightSwChanged(int id, bool sw)
{
    qDebug() << "DataContext::onCarLightSwChanged - id:" << id << "sw:" << sw;
    if (id >= 0 && id < cl.size()) {
        if (cl.at(id)->isLightOn != sw) {
            cl.at(id)->isLightOn = sw;
            qDebug() << "  -> Updated car" << id << "light to" << sw;
            emit dataChanged();  // 确保发送数据变化信号
        }
    }
}

void DataContext::onCarAlarmSwChanged(int id, bool sw)
{
    if (id >= 0 && id < cl.size()) {
        if (cl.at(id)->isAlarmOn != sw) {
            cl.at(id)->isAlarmOn = sw;
            emit dataChanged();
        }
    }
}

void DataContext::onCarRunSwChanged(int id, bool sw)
{
    if (id >= 0 && id < cl.size()) {
        if (cl.at(id)->isRunning != sw) {
            cl.at(id)->isRunning = sw;
            if (id < carMoving.size()) {
                carMoving[id] = sw;
            }
            emit dataChanged();
        }
    }
}

void DataContext::onCarLockSwChanged(int id, bool sw)
{
    if (id >= 0 && id < cl.size()) {
        if (cl.at(id)->isLocked != sw) {
            cl.at(id)->isLocked = sw;
            emit dataChanged();
        }
    }
}

void DataContext::setCarEnduranceMileage(int id, double endurance, double mileage)
{
    if (id >= 0 && id < cl.size()) {
        qDebug() << "setCarEnduranceMileage - id:" << id
                 << "endurance:" << endurance << "mileage:" << mileage;
        cl[id]->endurance = endurance;
        cl[id]->mileage   = mileage;
        emit dataChanged();
    } else {
        qDebug() << "setCarEnduranceMileage - invalid id:" << id;
    }
}

void DataContext::setCarRunPos(int id, int roadId, int roadPos)
{
    if(id >= cl.size())
        return;
    int oldRoadId = cl.at(id)->roadId;
    cl.at(id)->roadId = roadId;
    cl.at(id)->roadPos = roadPos;

    if(oldRoadId != roadId) {
        cl[id]->isWaitingAtRedLight = false;
        cl[id]->waitingLightId = -1;
        autoSetRoadColor(oldRoadId);
        autoSetRoadColor(roadId);
    }

    emit dataChanged();
}

RoadInfo::RoadInfo(int x, int y, int w, int h, int dir)
    : x{x}, y{y}, w{w}, h{h}, dir{dir}
{
    color = 0;
}

LightInfo::LightInfo(int roadId)
    : roadId{roadId}
{
    color = 0;
}

CarInfo::CarInfo()
    : roadId(-1)
    , roadPos(-1)
    , isRunning(false)
    , isLightOn(false)
    , isAlarmOn(false)
    , isLocked(false)
    , isWaitingAtRedLight(false)
    , waitingLightId(-1)
    , endurance(100.0)
    , mileage(0.0)
{
}