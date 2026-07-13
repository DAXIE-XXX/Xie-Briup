#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpServer>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QSet>
#include <QDateTime>
#include <QTimer>
#include "threshold.h"

class ThreadsHandle;
class MyTcpSocket;

// 房间联动触发状态：追踪哪些传感器当前超标，用于判断设备自动关闭时机
struct RoomTriggerState {
    QSet<QString> activeFanTriggers;   // 当前触发风扇的传感器（风扇自动关判断用）
    QSet<QString> activeAlarmTriggers; // 当前触发报警器/灯且可自动关的传感器
    QSet<QString> emergencySensors;    // 当前活跃的紧急传感器（仅手动关类型）
    QDateTime lastFanAbnormalTime;     // 最后一次风扇相关异常的时间
    QDateTime lastAlarmAbnormalTime;   // 最后一次报警器相关异常的时间
    bool fanRunning = false;           // 风扇是否已开启
    bool alarmRunning = false;         // 报警器/灯是否已开启
};

class MyTcpServer : public QTcpServer
{
    friend class NetContext;
    Q_OBJECT

signals:
    void forwardToGateway(const QJsonObject &obj);

public slots:
    void broadcastToClients(const QJsonObject &data);

public:
    // ===== 新增：设备管理 =====
    void registerDevice(MyTcpSocket *socket, int roomId, bool isGateway);
    int getRoomIdBySocket(MyTcpSocket *socket);
    Threshold getThreshold(int roomId);
    void setThreshold(int roomId, const Threshold &th);
    void sendToGateway(int roomId, const QJsonObject &cmd);
    void sendToAllGateways(const QJsonObject &cmd);
    bool hasGatewayForRoom(int roomId);

    // 房间触发状态管理（风扇自动关闭判断）
    void addFanTrigger(int roomId, const QString &sensorType);
    void removeFanTrigger(int roomId, const QString &sensorType);
    bool shouldTurnOffFan(int roomId, int fanCooldownSeconds);
    bool isFanRunning(int roomId);
    void setFanRunning(int roomId, bool running);

    // 房间触发状态管理（报警器/灯自动关闭判断）
    void addAlarmTrigger(int roomId, const QString &sensorType);
    void removeAlarmTrigger(int roomId, const QString &sensorType);
    void addEmergencySensor(int roomId, const QString &sensorType);
    void removeEmergencySensor(int roomId, const QString &sensorType);
    void clearFanTriggers(int roomId);
    void clearAlarmTriggers(int roomId);
    void clearEmergencySensors(int roomId);
    bool shouldTurnOffAlarm(int roomId, int cooldownSeconds);
    bool isAlarmRunning(int roomId);
    void setAlarmRunning(int roomId, bool running);

protected:
    explicit MyTcpServer(QObject *parent = nullptr);

protected slots:
    void socketDisconnectedSlot();

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    int limit = 500;
    int count = 0;
    ThreadsHandle *tsh;
    QList<MyTcpSocket*> clients;

    // ===== 新增：设备映射 =====
    QMap<MyTcpSocket*, int> socketRoomMap;        // socket -> roomId
    QMap<int, MyTcpSocket*> roomGatewayMap;       // roomId -> 网关socket
    QMap<int, Threshold> roomThresholds;          // roomId -> 阈值配置
    QMap<int, QDateTime> lastTriggerTime;         // roomId -> 上次触发时间（冷却用）
    QMap<int, RoomTriggerState> roomTriggerStates; // roomId -> 房间触发状态（风扇自动关闭判断）
    QMutex mapMutex;

    // ===== 新增：从socket获取socket描述符（用于查找） =====
    MyTcpSocket* findSocketByDescriptor(qintptr descriptor);
};

#endif // MYTCPSERVER_H