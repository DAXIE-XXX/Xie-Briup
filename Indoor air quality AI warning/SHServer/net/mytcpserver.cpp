#include "mytcpserver.h"
#include "mytcpsocket.h"
#include "threadshandle.h"
#include <QThread>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>

MyTcpServer::MyTcpServer(QObject *parent)
    : QTcpServer{parent}
    , tsh{new ThreadsHandle(this)}
{}

void MyTcpServer::socketDisconnectedSlot()
{
    MyTcpSocket *socket = qobject_cast<MyTcpSocket *>(sender());
    if (!socket) return;

    count--;
    clients.removeOne(socket);

    // ===== 新增：清理映射 =====
    mapMutex.lock();
    int roomId = socketRoomMap.value(socket, -1);
    if (roomId != -1) {
        socketRoomMap.remove(socket);
        // 如果这个房间的网关断开了，清空映射
        if (roomGatewayMap.value(roomId) == socket) {
            roomGatewayMap.remove(roomId);
        }
    }
    mapMutex.unlock();

    disconnect(socket, nullptr, nullptr, nullptr);
    disconnect(this, nullptr, socket, nullptr);
    QThread *th = socket->thread();
    socket->deleteLater();
    tsh->releaseTh(th);
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    MyTcpSocket *socket = new MyTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);
    socket->setServer(this);   // 设置服务器引用（因 moveToThread 后 parent() 为 null）

    if(count >= limit){
        socket->disconnectFromHost();
        socket->deleteLater();
        return;
    }
    count++;
    clients.append(socket);

    connect(socket, &MyTcpSocket::forwardToGateway,
            this, &MyTcpServer::forwardToGateway);
    connect(this,
            &MyTcpServer::forwardToGateway,
            socket,
            &MyTcpSocket::forwardToGatewaySlot);

    connect(socket, &MyTcpSocket::sensorDataReceived,
            this, &MyTcpServer::broadcastToClients);

    connect(socket, &MyTcpSocket::disconnected,
            this, &MyTcpServer::socketDisconnectedSlot);

    // ===== 新增：连接设备注册信号 =====
    connect(socket, &MyTcpSocket::deviceRegistered,
            this, &MyTcpServer::registerDevice);

    QThread *th = tsh->getTh();
    socket->moveToThread(th);
}

void MyTcpServer::broadcastToClients(const QJsonObject &data)
{
    QByteArray jsonData = QJsonDocument(data).toJson(QJsonDocument::Compact);
    jsonData.append('\n');
    for(MyTcpSocket *client : clients){
        if(!client->gateway && client->state() == QTcpSocket::ConnectedState){
            client->write(jsonData);
            client->flush();
        }
    }
}

// ==================== 新增实现 ====================

void MyTcpServer::registerDevice(MyTcpSocket *socket, int roomId, bool isGateway)
{
    if (!socket) return;

    mapMutex.lock();

    // 先清理旧映射
    int oldRoomId = socketRoomMap.value(socket, -1);
    if (oldRoomId != -1) {
        socketRoomMap.remove(socket);
        if (roomGatewayMap.value(oldRoomId) == socket) {
            roomGatewayMap.remove(oldRoomId);
        }
    }

    // 注册新映射
    socketRoomMap[socket] = roomId;
    if (isGateway) {
        roomGatewayMap[roomId] = socket;
        qDebug() << "✅ 网关注册: roomId=" << roomId;
    } else {
        qDebug() << "✅ 传感器注册: roomId=" << roomId;
    }

    mapMutex.unlock();
}

int MyTcpServer::getRoomIdBySocket(MyTcpSocket *socket)
{
    mapMutex.lock();
    int roomId = socketRoomMap.value(socket, -1);
    mapMutex.unlock();
    return roomId;
}

Threshold MyTcpServer::getThreshold(int roomId)
{
    mapMutex.lock();
    Threshold th = roomThresholds.value(roomId, Threshold());
    mapMutex.unlock();
    return th;
}

void MyTcpServer::setThreshold(int roomId, const Threshold &th)
{
    mapMutex.lock();
    roomThresholds[roomId] = th;
    mapMutex.unlock();
    qDebug() << "📊 阈值更新: roomId=" << roomId
             << " CO2=" << th.co2Max
             << " PM2.5=" << th.pm25Max
             << " TempMax=" << th.tempMax;
}

void MyTcpServer::sendToGateway(int roomId, const QJsonObject &cmd)
{
    mapMutex.lock();
    MyTcpSocket *gw = roomGatewayMap.value(roomId, nullptr);
    mapMutex.unlock();

    if (!gw) {
        qWarning() << "⚠️ 房间" << roomId << "没有网关在线";
        return;
    }

    if (gw->state() == QTcpSocket::ConnectedState) {
        QByteArray data = QJsonDocument(cmd).toJson(QJsonDocument::Compact);
        data.append('\n');
        gw->write(data);
        gw->flush();
        qDebug() << "📤 发送指令到网关 roomId=" << roomId << ":" << cmd;
    } else {
        qWarning() << "⚠️ 网关已断开 roomId=" << roomId;
    }
}

void MyTcpServer::sendToAllGateways(const QJsonObject &cmd)
{
    mapMutex.lock();
    QList<MyTcpSocket*> gateways = roomGatewayMap.values();
    mapMutex.unlock();

    QByteArray data = QJsonDocument(cmd).toJson(QJsonDocument::Compact);
    data.append('\n');

    for (MyTcpSocket *gw : gateways) {
        if (gw && gw->state() == QTcpSocket::ConnectedState) {
            gw->write(data);
            gw->flush();
        }
    }
    qDebug() << "📤 广播指令到所有网关:" << cmd;
}

bool MyTcpServer::hasGatewayForRoom(int roomId)
{
    mapMutex.lock();
    bool has = roomGatewayMap.contains(roomId);
    mapMutex.unlock();
    return has;
}

// ==================== 房间触发状态管理（风扇自动关闭判断） ====================

void MyTcpServer::addFanTrigger(int roomId, const QString &sensorType)
{
    mapMutex.lock();
    RoomTriggerState &state = roomTriggerStates[roomId];
    state.activeFanTriggers.insert(sensorType);
    state.lastFanAbnormalTime = QDateTime::currentDateTime();
    mapMutex.unlock();
}

void MyTcpServer::removeFanTrigger(int roomId, const QString &sensorType)
{
    mapMutex.lock();
    if (roomTriggerStates.contains(roomId)) {
        roomTriggerStates[roomId].activeFanTriggers.remove(sensorType);
    }
    mapMutex.unlock();
}

bool MyTcpServer::shouldTurnOffFan(int roomId, int fanCooldownSeconds)
{
    mapMutex.lock();
    if (!roomTriggerStates.contains(roomId)) {
        mapMutex.unlock();
        return false;
    }
    RoomTriggerState &state = roomTriggerStates[roomId];
    bool allClear = state.activeFanTriggers.isEmpty();
    bool cooldownPassed = false;
    if (allClear && state.lastFanAbnormalTime.isValid()) {
        int elapsed = state.lastFanAbnormalTime.secsTo(QDateTime::currentDateTime());
        cooldownPassed = (elapsed >= fanCooldownSeconds);
    }
    mapMutex.unlock();
    return allClear && cooldownPassed;
}

bool MyTcpServer::isFanRunning(int roomId)
{
    mapMutex.lock();
    bool running = roomTriggerStates.value(roomId).fanRunning;
    mapMutex.unlock();
    return running;
}

void MyTcpServer::setFanRunning(int roomId, bool running)
{
    mapMutex.lock();
    roomTriggerStates[roomId].fanRunning = running;
    mapMutex.unlock();
}

// ==================== 房间触发状态管理（报警器/灯自动关闭判断） ====================

void MyTcpServer::addAlarmTrigger(int roomId, const QString &sensorType)
{
    mapMutex.lock();
    RoomTriggerState &state = roomTriggerStates[roomId];
    state.activeAlarmTriggers.insert(sensorType);
    state.lastAlarmAbnormalTime = QDateTime::currentDateTime();
    mapMutex.unlock();
}

void MyTcpServer::removeAlarmTrigger(int roomId, const QString &sensorType)
{
    mapMutex.lock();
    if (roomTriggerStates.contains(roomId)) {
        roomTriggerStates[roomId].activeAlarmTriggers.remove(sensorType);
    }
    mapMutex.unlock();
}

void MyTcpServer::addEmergencySensor(int roomId, const QString &sensorType)
{
    mapMutex.lock();
    roomTriggerStates[roomId].emergencySensors.insert(sensorType);
    mapMutex.unlock();
}

void MyTcpServer::removeEmergencySensor(int roomId, const QString &sensorType)
{
    mapMutex.lock();
    if (roomTriggerStates.contains(roomId)) {
        roomTriggerStates[roomId].emergencySensors.remove(sensorType);
    }
    mapMutex.unlock();
}

void MyTcpServer::clearFanTriggers(int roomId)
{
    mapMutex.lock();
    if (roomTriggerStates.contains(roomId)) {
        roomTriggerStates[roomId].activeFanTriggers.clear();
    }
    mapMutex.unlock();
}

void MyTcpServer::clearAlarmTriggers(int roomId)
{
    mapMutex.lock();
    if (roomTriggerStates.contains(roomId)) {
        roomTriggerStates[roomId].activeAlarmTriggers.clear();
    }
    mapMutex.unlock();
}

void MyTcpServer::clearEmergencySensors(int roomId)
{
    mapMutex.lock();
    if (roomTriggerStates.contains(roomId)) {
        roomTriggerStates[roomId].emergencySensors.clear();
    }
    mapMutex.unlock();
}

bool MyTcpServer::shouldTurnOffAlarm(int roomId, int cooldownSeconds)
{
    mapMutex.lock();
    if (!roomTriggerStates.contains(roomId)) {
        mapMutex.unlock();
        return false;
    }
    RoomTriggerState &state = roomTriggerStates[roomId];
    // 自动关条件：可自动关的传感器全部恢复 + 没有紧急传感器活跃 + 延迟已过
    bool allAutoClear = state.activeAlarmTriggers.isEmpty();
    bool noEmergency = state.emergencySensors.isEmpty();
    bool cooldownPassed = false;
    if (allAutoClear && noEmergency && state.lastAlarmAbnormalTime.isValid()) {
        int elapsed = state.lastAlarmAbnormalTime.secsTo(QDateTime::currentDateTime());
        cooldownPassed = (elapsed >= cooldownSeconds);
    }
    mapMutex.unlock();
    return allAutoClear && noEmergency && cooldownPassed;
}

bool MyTcpServer::isAlarmRunning(int roomId)
{
    mapMutex.lock();
    bool running = roomTriggerStates.value(roomId).alarmRunning;
    mapMutex.unlock();
    return running;
}

void MyTcpServer::setAlarmRunning(int roomId, bool running)
{
    mapMutex.lock();
    roomTriggerStates[roomId].alarmRunning = running;
    mapMutex.unlock();
}