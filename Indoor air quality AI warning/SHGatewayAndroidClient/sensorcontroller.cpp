#include "sensorcontroller.h"
#include "tcpclient.h"
#include <QDebug>
#include <QJsonArray>

SensorController::SensorController(QObject *parent)
    : QObject(parent)
    , m_tcpClient(new TcpClient(this))
{
    // TCP 客户端信号 → 控制器槽
    connect(m_tcpClient, &TcpClient::connected,
            this, &SensorController::onConnected);
    connect(m_tcpClient, &TcpClient::disconnected,
            this, &SensorController::onDisconnected);
    connect(m_tcpClient, &TcpClient::errorOccurred,
            this, &SensorController::onError);
    connect(m_tcpClient, &TcpClient::dataReceived,
            this, &SensorController::onDataReceived);
    connect(m_tcpClient, &TcpClient::serverCommandReceived,
            this, &SensorController::onServerCommand);
}

SensorController::~SensorController() = default;

// ==================== 属性访问 ====================

bool SensorController::isConnected() const { return m_tcpClient->isConnected(); }
QString SensorController::serverIp() const { return m_serverIp; }
int SensorController::serverPort() const { return m_serverPort; }

double SensorController::temperature() const { return m_temperature; }
double SensorController::humidity() const    { return m_humidity; }
double SensorController::co2() const         { return m_co2; }
double SensorController::pm25() const        { return m_pm25; }
bool   SensorController::methane() const     { return m_methane; }
double SensorController::light() const       { return m_light; }
bool   SensorController::smoke() const       { return m_smoke; }
bool   SensorController::fire() const        { return m_fire; }

int    SensorController::co2Max() const          { return m_co2Max; }
int    SensorController::pm25Max() const         { return m_pm25Max; }
double SensorController::tempMax() const          { return m_tempMax; }
double SensorController::tempMin() const          { return m_tempMin; }
bool   SensorController::methaneEnable() const    { return m_methaneEnable; }
int    SensorController::cooldownSeconds() const  { return m_cooldownSeconds; }
int    SensorController::fanCooldownSeconds() const { return m_fanCooldownSeconds; }

// ==================== QML 可调用方法 ====================

void SensorController::connectToServer(const QString &ip, int port)
{
    m_serverIp = ip;
    m_serverPort = port;
    emit serverIpChanged();
    emit serverPortChanged();
    m_tcpClient->connectToServer(ip, static_cast<quint16>(port));
}

void SensorController::disconnectFromServer()
{
    m_tcpClient->disconnectFromServer();
}

void SensorController::requestSensor(int sensorType)
{
    // 发送传感器读取请求 (20001~20008)
    QJsonObject cmd;
    cmd["type"] = sensorType;
    m_tcpClient->sendCommand(cmd);
}

void SensorController::sendControl(int controlType, const QVariantMap &params)
{
    QJsonObject cmd;
    cmd["type"] = controlType;
    for (auto it = params.begin(); it != params.end(); ++it) {
        cmd[it.key()] = QJsonValue::fromVariant(it.value());
    }
    m_tcpClient->sendCommand(cmd);
    qDebug() << "📤 发送控制指令:" << cmd;
}

void SensorController::applyThreshold(int co2Max, int pm25Max, double tempMax,
                                       double tempMin, bool methaneEnable,
                                       int cooldownSeconds, int fanCooldownSeconds)
{
    // 更新本地缓存
    m_co2Max = co2Max;
    m_pm25Max = pm25Max;
    m_tempMax = tempMax;
    m_tempMin = tempMin;
    m_methaneEnable = methaneEnable;
    m_cooldownSeconds = cooldownSeconds;
    m_fanCooldownSeconds = fanCooldownSeconds;
    emit thresholdChanged();

    // 下发给服务器
    QJsonObject cmd;
    cmd["type"] = 30001;
    cmd["room_id"] = m_roomId;
    cmd["co2_max"] = co2Max;
    cmd["pm25_max"] = pm25Max;
    cmd["temp_max"] = tempMax;
    cmd["temp_min"] = tempMin;
    cmd["methane_enable"] = methaneEnable;
    cmd["cooldown_seconds"] = cooldownSeconds;
    cmd["fan_cooldown_seconds"] = fanCooldownSeconds;
    m_tcpClient->sendCommand(cmd);
    qDebug() << "📊 下发阈值配置:" << cmd;
}

void SensorController::queryHistory(int sensorType, const QString &range)
{
    // 连接检查：未连接时通知 QML 显示错误提示
    if (!m_tcpClient->isConnected()) {
        qWarning() << "⚠️ 查询历史失败: 未连接到服务器";
        emit historyQueryError("未连接到服务器，请先连接");
        return;
    }

    QJsonObject cmd;
    cmd["type"] = 40001;
    cmd["sensor_type"] = sensorType;
    cmd["range"] = range;
    m_tcpClient->sendCommand(cmd);
    qDebug() << "📊 查询历史: sensorType=" << sensorType << " range=" << range;
}

// ==================== 内部槽函数 ====================

void SensorController::onConnected()
{
    qDebug() << "✅ 控制器: 已连接服务器";
    // 发送设备注册（作为客户端）
    QJsonObject reg;
    reg["type"] = 1;
    reg["room_id"] = m_roomId;
    reg["is_gateway"] = false;
    reg["device_id"] = QString("android_client_%1").arg(m_roomId);
    m_tcpClient->sendCommand(reg);

    // 延迟发送阈值配置
    QTimer::singleShot(500, this, [this]() {
        applyThreshold(m_co2Max, m_pm25Max, m_tempMax, m_tempMin,
                       m_methaneEnable, m_cooldownSeconds, m_fanCooldownSeconds);
    });

    emit connectedChanged();
}

void SensorController::onDisconnected()
{
    qDebug() << "❌ 控制器: 连接断开";
    emit connectedChanged();
}

void SensorController::onError(const QString &error)
{
    qWarning() << "⚠️ 控制器: 连接错误" << error;
    emit connectionError(error);
    emit connectedChanged();
}

void SensorController::onDataReceived(const QJsonObject &data)
{
    int type = data.value("type").toInt();

    // 传感器数据 (10001~10008)
    if (type >= 10001 && type <= 10008) {
        double value = data.value("value").toDouble();
        updateSensorValue(type, value);
    }

    // 阈值确认 (30002)
    if (type == 30002) {
        bool success = data.value("success").toBool();
        emit thresholdApplied(success);
        qDebug() << "📊 阈值确认:" << (success ? "成功" : "失败");
    }

    // 历史数据查询响应 (40002)
    if (type == 40002) {
        int sensorType = data.value("sensor_type").toInt();
        QString range = data.value("range").toString();
        QJsonArray historyData = data.value("data").toArray();
        // 序列化为 JSON 字符串传递，QML 端用 JSON.parse 解析，避免跨边界类型转换问题
        QJsonDocument doc(historyData);
        QString dataJson = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        emit historyReceived(sensorType, range, dataJson);
        qDebug() << "📊 收到历史数据: sensorType=" << sensorType
                 << " 记录数=" << historyData.size()
                 << " JSON=" << dataJson;
    }
}

void SensorController::onServerCommand(const QJsonObject &cmd)
{
    // 告警通知 (20050)
    int type = cmd.value("type").toInt();
    if (type == 20050) {
        QString action = cmd.value("action").toString();
        double value = cmd.value("value").toDouble();
        QString timestamp = cmd.value("timestamp").toString();
        emit alertReceived(action, value, timestamp);
        qDebug() << "🚨 收到告警:" << action;
    }
}

// ==================== 内部辅助方法 ====================

void SensorController::updateSensorValue(int sensorType, double value)
{
    switch (sensorType) {
    case 10001: m_co2 = value;         break;
    case 10002: m_temperature = value;  break;
    case 10003: m_fire = (value > 0);   break;
    case 10004: m_pm25 = value;         break;
    case 10005: m_methane = (value > 0); break;
    case 10006: m_light = value;        break;
    case 10007: m_smoke = (value > 0);  break;
    case 10008: m_humidity = value;     break;
    default: return;
    }
    emit dataChanged();
}
