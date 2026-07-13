#include "mytcpsocket.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QList>
#include <QDebug>
#include "dbcontext.h"
#include "dbexec.h"
#include <QCryptographicHash>
#include <QDateTime>
#include "mytcpserver.h"  // 新增

// ... 现有代码保持不变 ...

void MyTcpSocket::forwardToGatewaySlot(const QJsonObject &obj)
{
    if(!gateway)
        return;
    this->write(QJsonDocument(obj).toJson());
}

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, &MyTcpSocket::readyRead,
            this, &MyTcpSocket::readyReadSlot);
}

void MyTcpSocket::readyReadSlot()
{
    // 追加新数据到持久化缓冲区，防止TCP分帧导致数据丢失
    m_buffer.append(this->readAll());
    handleData();
}

void MyTcpSocket::handleData()
{
    // 从持久化缓冲区解析完整JSON帧（按{}括号匹配）
    while (true) {
        int count = 0;
        int frameEnd = -1;
        for (int i = 0; i < m_buffer.length(); i++) {
            if (m_buffer.at(i) == '{')
                count++;
            else if (m_buffer.at(i) == '}') {
                count--;
                if (count == 0) {
                    frameEnd = i;
                    break;
                }
            }
        }
        if (frameEnd == -1)
            break;  // 没有完整帧，等待更多数据

        QByteArray frame = m_buffer.mid(0, frameEnd + 1);
        m_buffer.remove(0, frameEnd + 1);  // 从缓冲区移除已处理的数据
        handleFrame(frame);
    }
}

void MyTcpSocket::handleFrame(const QByteArray &frame)
{
    QJsonObject rf = QJsonDocument::fromJson(frame).object();
    int type = rf.value("type").toInt();

    // ===== 设备注册 (type=1) =====
    if (type == 1) {
        handleRegister(rf);
        return;
    }

    // ===== 阈值设置 (type=30001) =====
    if (type == 30001) {
        int roomId = rf.value("room_id").toInt();
        if (m_server) {
            Threshold th;
            th.co2Max = rf.value("co2_max").toInt(1000);
            th.pm25Max = rf.value("pm25_max").toInt(50);
            th.tempMax = rf.value("temp_max").toDouble(30.0);
            th.tempMin = rf.value("temp_min").toDouble(18.0);
            th.methaneEnable = rf.value("methane_enable").toBool(true);
            th.cooldownSeconds = rf.value("cooldown_seconds").toInt(30);
            th.fanCooldownSeconds = rf.value("fan_cooldown_seconds").toInt(30);
            m_server->setThreshold(roomId, th);

            qDebug() << "📊 收到阈值设置: roomId=" << roomId
                     << " CO2=" << th.co2Max
                     << " PM2.5=" << th.pm25Max
                     << " 风扇延迟=" << th.fanCooldownSeconds << "s";

            // 回复确认
            QJsonObject reply;
            reply["type"] = 30002;
            reply["room_id"] = roomId;
            reply["success"] = true;
            write(QJsonDocument(reply).toJson(QJsonDocument::Compact) + "\n");
        }
        return;
    }

    // ===== 客户端请求传感器数据 (20001~20008) =====
    // 转发给网关，由网关触发实际传感器硬件读取
    // 网关读取硬件后会上报 10001~10008，服务器再广播给所有客户端
    if (type >= 20001 && type <= 20008) {
        emit forwardToGateway(rf);
        return;
    }

    // ===== 网关控制指令 (20011~20015等) =====
    if (type >= 20011 && type < 30000) {
        emit forwardToGateway(rf);

        // 手动关闭设备时同步服务器状态（支持超标期间手动关 + 冷却后重新检测）
        int roomId = m_server ? m_server->getRoomIdBySocket(this) : -1;
        if (roomId != -1) {
            if (type == 20013 && !rf.value("sw").toBool()) {
                // 手动关风扇：重置状态，冷却后超标可重新触发
                m_server->clearFanTriggers(roomId);
                m_server->setFanRunning(roomId, false);
                qDebug() << "  [状态同步] 风扇手动关闭，清除追踪状态";
            }
            if ((type == 20014 || type == 20015) && !rf.value("sw").toBool()) {
                // 手动关报警器/灯：清除紧急集合，当前事件不再触发，恢复后可重新检测
                m_server->clearEmergencySensors(roomId);
                m_server->clearAlarmTriggers(roomId);
                m_server->setAlarmRunning(roomId, false);
                qDebug() << "  [状态同步] 报警器/灯手动关闭，清除追踪状态";
            }
        }
        return;
    }

    // ===== 传感器数据上报 (10000~19999) =====
    if (type >= 10000 && type < 20000) {
        gateway = true;
    }

    // ===== 传感器数据上报处理（增加联动检查） =====
    QString currentTime = QDateTime::currentDateTime()
                              .toString("yyyy-MM-dd hh:mm:ss");

    bool isSensorData = false;
    switch(type) {
    case 10001: {
        int value = rf.value("value").toInt();
        DBExec *exec = DBContext::getObject()->getDBExec();
        if(exec != nullptr) {
            bool ok = exec->sensorInsertCo2(value, currentTime);
            if(!ok) {
                qWarning() << "Failed to insert CO2 data:" << value;
            }
            DBContext::getObject()->releaseDBExec(exec);
        }
        isSensorData = true;
        checkAndTrigger("co2", value);
        break;
    }
    case 10002: {
        double value = rf.value("value").toDouble();
        DBExec *exec = DBContext::getObject()->getDBExec();
        if(exec != nullptr) {
            bool ok = exec->sensorInsertTemperature(value, currentTime);
            if(!ok) {
                qWarning() << "Failed to insert Temperature data:" << value;
            }
            DBContext::getObject()->releaseDBExec(exec);
        }
        isSensorData = true;
        checkAndTrigger("temp", value);
        break;
    }
    case 10003: {
        bool value = rf.value("value").toBool();
        DBExec *exec = DBContext::getObject()->getDBExec();
        if(exec != nullptr) {
            bool ok = exec->sensorInsertFire(value, currentTime);
            if(!ok) {
                qWarning() << "Failed to insert Fire data:" << value;
            }
            DBContext::getObject()->releaseDBExec(exec);
        }
        isSensorData = true;
        if (value) {
            checkAndTrigger("fire", 1);
        }
        break;
    }
    case 10004: {
        int value = rf.value("value").toInt();
        DBExec *exec = DBContext::getObject()->getDBExec();
        if(exec != nullptr) {
            bool ok = exec->sensorInsertPM25(value, currentTime);
            if(!ok) {
                qWarning() << "Failed to insert PM2.5 data:" << value;
            }
            DBContext::getObject()->releaseDBExec(exec);
        }
        isSensorData = true;
        checkAndTrigger("pm25", value);
        break;
    }
    case 10005: {
        bool value = rf.value("value").toBool();
        DBExec *exec = DBContext::getObject()->getDBExec();
        if(exec != nullptr) {
            bool ok = exec->sensorInsertMethane(value, currentTime);
            if(!ok) {
                qWarning() << "Failed to insert Methane data:" << value;
            }
            DBContext::getObject()->releaseDBExec(exec);
        }
        isSensorData = true;
        if (value) {
            checkAndTrigger("methane", 1);
        }
        break;
    }
    case 10006: {
        int value = rf.value("value").toInt();
        DBExec *exec = DBContext::getObject()->getDBExec();
        if(exec != nullptr) {
            bool ok = exec->sensorInsertLight(value, currentTime);
            if(!ok) {
                qWarning() << "Failed to insert Light data:" << value;
            }
            DBContext::getObject()->releaseDBExec(exec);
        }
        isSensorData = true;
        break;
    }
    case 10007: {
        bool value = rf.value("value").toBool();
        DBExec *exec = DBContext::getObject()->getDBExec();
        if(exec != nullptr) {
            bool ok = exec->sensorInsertSmoke(value, currentTime);
            if(!ok) {
                qWarning() << "Failed to insert Smoke data:" << value;
            }
            DBContext::getObject()->releaseDBExec(exec);
        }
        isSensorData = true;
        if (value) {
            checkAndTrigger("smoke", 1);
        }
        break;
    }
    case 10008: {
        double value = rf.value("value").toDouble();
        DBExec *exec = DBContext::getObject()->getDBExec();
        if(exec != nullptr) {
            bool ok = exec->sensorInsertHumidity(value, currentTime);
            if(!ok) {
                qWarning() << "Failed to insert Humidity data:" << value;
            }
            DBContext::getObject()->releaseDBExec(exec);
        }
        isSensorData = true;
        break;
    }
    // ===== 历史数据查询 (40001) =====
    case 40001: {
        int sensorType = rf.value("sensor_type").toInt();
        QString range = rf.value("range").toString("24h");
        DBExec *exec = DBContext::getObject()->getDBExec();
        QJsonArray history;
        if (exec != nullptr) {
            history = exec->sensorQueryHistory(sensorType, range);
            DBContext::getObject()->releaseDBExec(exec);
        }
        QJsonObject reply;
        reply["type"] = 40002;
        reply["sensor_type"] = sensorType;
        reply["range"] = range;
        reply["data"] = history;
        write(QJsonDocument(reply).toJson(QJsonDocument::Compact) + "\n");
        qDebug() << "📊 历史查询: sensorType=" << sensorType
                 << " range=" << range << " 记录数=" << history.size();
        return;
    }
    default: {
        // 只对非20001~20008的未知类型打印警告
        if (type < 20001 || type > 20008) {
            qWarning() << "Unknown sensor type:" << type;
        }
        break;
    }
    }

    if(isSensorData) {
        emit sensorDataReceived(rf);
    }
}
// ===== 新增：设备注册处理 =====
void MyTcpSocket::handleRegister(const QJsonObject &obj)
{
    int roomId = obj.value("room_id").toInt();
    bool isGateway = obj.value("is_gateway").toBool(false);
    QString deviceId = obj.value("device_id").toString();

    this->roomId = roomId;
    this->deviceId = deviceId;

    // 如果是网关设备，标记gateway标志
    if (isGateway) {
        this->gateway = true;
    }

    // 通知服务器注册
    emit deviceRegistered(this, roomId, isGateway);

    qDebug() << "📱 设备注册: roomId=" << roomId
             << " deviceId=" << deviceId
             << " isGateway=" << isGateway;
}

// ===== 联动检查：传感器超标时向网关下发控制指令 =====
//
// 联动策略：
//   CO2/PM2.5超标 → 风扇(自动关) + 报警灯(自动关)，超标期间可手动关，冷却后重新检测触发
//   甲烷/烟雾/火光 → 报警器+报警灯(仅手动关)，手动关后当前事件不再触发，恢复正常后重新检测
//
void MyTcpSocket::checkAndTrigger(const QString &type, double value)
{
    if (!m_server) return;

    int roomId = m_server->getRoomIdBySocket(this);
    if (roomId == -1) {
        qDebug() << "⚠️ 设备未注册房间，跳过联动检查";
        return;
    }

    if (!m_server->hasGatewayForRoom(roomId)) {
        qDebug() << "⚠️ 房间" << roomId << "没有网关在线，跳过联动";
        return;
    }

    Threshold th = m_server->getThreshold(roomId);
    QDateTime now = QDateTime::currentDateTime();

    // ===== 判定当前传感器是否超标 =====
    bool isAbnormal = false;
    bool needFan = false;          // 需开风扇（仅CO2/PM2.5）
    bool needAlarmLight = false;   // 需开报警灯
    bool needAlarmBuzzer = false;  // 需开报警器蜂鸣
    bool autoOff = false;          // 是否自动关（CO2/PM2.5=true，其他=false）
    QString reason;

    // ---- 类型A：CO2/PM2.5 → 风扇+报警灯，自动关，可手动关 ----
    if (type == "co2" && value > th.co2Max) {
        isAbnormal = true;
        needFan = true;
        needAlarmLight = true;
        autoOff = true;
        reason = QString("CO₂超标: %.0f ppm > %d ppm").arg(value).arg(th.co2Max);
    }
    else if (type == "pm25" && value > th.pm25Max) {
        isAbnormal = true;
        needFan = true;
        needAlarmLight = true;
        autoOff = true;
        reason = QString("PM2.5超标: %.0f µg/m³ > %d µg/m³").arg(value).arg(th.pm25Max);
    }
    // ---- 类型B：甲烷/烟雾/火光 → 报警器+报警灯，仅手动关 ----
    else if (type == "methane" && value > 0 && th.methaneEnable) {
        isAbnormal = true;
        needAlarmLight = true;
        needAlarmBuzzer = true;
        // autoOff = false（默认）：仅手动关
        reason = QString("⚠️ 甲烷泄漏检测！");
    }
    else if (type == "fire" && value > 0) {
        isAbnormal = true;
        needAlarmLight = true;
        needAlarmBuzzer = true;
        reason = QString("🔥 检测到火焰！");
    }
    else if (type == "smoke" && value > 0) {
        isAbnormal = true;
        needAlarmLight = true;
        needAlarmBuzzer = true;
        reason = QString("💨 检测到烟雾！");
    }

    // ===== 情形一：数值超标 → 开启设备 =====
    if (isAbnormal) {
        // 每种传感器独立冷却
        if (lastTriggerTimes.contains(type)) {
            int elapsed = lastTriggerTimes[type].secsTo(now);
            if (elapsed < th.cooldownSeconds) {
                return;
            }
        }
        lastTriggerTimes[type] = now;

        qDebug() << "🚨 联动触发: roomId=" << roomId
                 << " type=" << type
                 << " reason=" << reason;

        // 风扇（仅CO2/PM2.5）
        if (needFan) {
            m_server->addFanTrigger(roomId, type);
            if (!m_server->isFanRunning(roomId)) {
                QJsonObject fanCmd;
                fanCmd.insert("type", 20013);
                fanCmd.insert("sw", true);
                m_server->sendToGateway(roomId, fanCmd);
                m_server->setFanRunning(roomId, true);
                qDebug() << "  → 网关: 打开风扇 (type=20013)";
            }
        }

        // 报警器/灯
        if (needAlarmLight || needAlarmBuzzer) {
            if (autoOff) {
                // 自动关类型：加入可自动关集合
                m_server->addAlarmTrigger(roomId, type);
            } else {
                // 仅手动关类型：加入紧急集合（阻止自动关）
                m_server->addEmergencySensor(roomId, type);
            }

            if (!m_server->isAlarmRunning(roomId)) {
                if (needAlarmLight) {
                    QJsonObject lightCmd;
                    lightCmd.insert("type", 20015);
                    lightCmd.insert("sw", true);
                    m_server->sendToGateway(roomId, lightCmd);
                }
                if (needAlarmBuzzer) {
                    QJsonObject alarmCmd;
                    alarmCmd.insert("type", 20014);
                    alarmCmd.insert("sw", true);
                    m_server->sendToGateway(roomId, alarmCmd);
                }
                m_server->setAlarmRunning(roomId, true);
                qDebug() << "  → 网关: 打开报警灯(type=20015)" << (needAlarmBuzzer ? "+报警器(type=20014)" : "");
            }
        }

        // 广播告警通知给所有客户端
        QJsonObject alert;
        alert.insert("type", 20050);
        alert.insert("action", reason);
        alert.insert("room_id", roomId);
        alert.insert("value", value);
        alert.insert("timestamp", now.toString(Qt::ISODate));
        m_server->broadcastToClients(alert);

        return;
    }

    // ===== 情形二：数值正常 → 移除触发标记，检查关闭设备 =====
    m_server->removeFanTrigger(roomId, type);
    m_server->removeAlarmTrigger(roomId, type);
    m_server->removeEmergencySensor(roomId, type);
    // 清除该传感器的冷却记录（恢复正常后下次超标可立即重新判定）
    lastTriggerTimes.remove(type);

    // 风扇自动关
    if (m_server->isFanRunning(roomId) &&
        m_server->shouldTurnOffFan(roomId, th.fanCooldownSeconds)) {
        QJsonObject fanOff;
        fanOff.insert("type", 20013);
        fanOff.insert("sw", false);
        m_server->sendToGateway(roomId, fanOff);
        m_server->setFanRunning(roomId, false);
        qDebug() << "  → 网关: 关闭风扇【延迟" << th.fanCooldownSeconds << "s自动关闭】";
    }

    // 报警器/灯自动关：需可自动关传感器全恢复 + 无紧急传感器 + 延迟已过
    if (m_server->isAlarmRunning(roomId) &&
        m_server->shouldTurnOffAlarm(roomId, th.fanCooldownSeconds)) {
        QJsonObject lightOff;
        lightOff.insert("type", 20015);
        lightOff.insert("sw", false);
        m_server->sendToGateway(roomId, lightOff);
        QJsonObject alarmOff;
        alarmOff.insert("type", 20014);
        alarmOff.insert("sw", false);
        m_server->sendToGateway(roomId, alarmOff);
        m_server->setAlarmRunning(roomId, false);
        qDebug() << "  → 网关: 关闭报警器+报警灯【延迟" << th.fanCooldownSeconds << "s自动关闭】";
    }
}

void MyTcpSocket::handleSensorRequest(int type)
{
    qDebug() << "📤 收到传感器请求: type=" << type;

    // 生成模拟数据（实际项目中这里应该从数据库或硬件读取）
    QJsonObject response;
    response["type"] = type + 10000;  // 20002 -> 10002 (温度数据)

    double value = 0;
    switch(type) {
    case 20001: // CO2
        value = 400 + (rand() % 600);
        response["value"] = value;
        break;
    case 20002: // 温度
        value = 20.0 + (rand() % 100) / 10.0;
        response["value"] = value;
        break;
    case 20003: // 火光
        response["value"] = (rand() % 10) > 8;
        break;
    case 20004: // PM2.5
        value = 10 + (rand() % 90);
        response["value"] = value;
        break;
    case 20005: // 甲烷
        response["value"] = (rand() % 10) > 8;
        break;
    case 20006: // 光照
        value = 100 + (rand() % 500);
        response["value"] = value;
        break;
    case 20007: // 烟雾
        response["value"] = (rand() % 10) > 8;
        break;
    case 20008: // 湿度
        value = 40.0 + (rand() % 300) / 10.0;
        response["value"] = value;
        break;
    default:
        qWarning() << "未知传感器请求类型:" << type;
        return;
    }

    // 发送响应给客户端
    QByteArray data = QJsonDocument(response).toJson(QJsonDocument::Compact);
    data.append('\n');
    this->write(data);
    this->flush();
    qDebug() << "📤 返回传感器数据:" << response;
}