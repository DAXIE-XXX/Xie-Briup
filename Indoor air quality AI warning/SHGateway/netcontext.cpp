#include "netcontext.h"

#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariant>
#include <QDebug>
#include "serialcontext.h"

/* ------------------------------------------------------------------ */
/* 单例实现 */
NetContext *NetContext::obj = nullptr;

NetContext::NetContext(QObject *parent)
    : QObject{parent}
    , client{new QTcpSocket(this)}
{
    connect(client, &QTcpSocket::readyRead,
            this, &NetContext::readyReadSlot);
}

/* ------------------------------------------------------------------ */
NetContext *NetContext::getObject()
{
    if (!obj) obj = new NetContext;
    return obj;
}

/* ------------------------------------------------------------------ */
bool NetContext::connectToServer(const QString &ip, quint16 port)
{
    // 保存连接参数，供自动重连使用
    m_lastIp = ip;
    m_lastPort = port;

    // 1) 已经连着旧的服务器，先断开
    if (client->state() == QTcpSocket::ConnectedState) {
        client->disconnectFromHost();                // 发送 DISCONNECT
        // 只有在真的处于 ConnectedState 时才等待
        if (client->state() != QTcpSocket::UnconnectedState)
            client->waitForDisconnected(1000);
    }

    // 2) 开始新的连接
    client->connectToHost(QHostAddress(ip), port);
    bool ok = client->waitForConnected(3000);
    if (!ok)
        qWarning() << "[NetContext] connect failed:" << client->errorString();
    else
        m_retryCount = 0;  // 连接成功，重置重试计数
    return ok;
}

/* ------------------------------------------------------------------ */
bool NetContext::isConnected() const
{
    return client->state() == QTcpSocket::ConnectedState;
}

/* ------------------------------------------------------------------ */
/** 向服务器注册网关设备，使服务器建立 roomId → 网关 的映射 */
void NetContext::registerToServer(int roomId)
{
    if (!isConnected()) {
        qWarning() << "[NetContext] 未连接服务器，无法注册";
        return;
    }
    QJsonObject reg;
    reg.insert("type", 1);
    reg.insert("room_id", roomId);
    reg.insert("is_gateway", true);   // 网关设备
    reg.insert("device_id", QString("gateway_%1").arg(roomId));
    sendJson(reg);
    qDebug() << "[NetContext] 网关注册: roomId=" << roomId;
}

/* ------------------------------------------------------------------ */
/*  私有的统一发送函数 – 把 JSON 编码后写入 socket               */
void NetContext::sendJson(const QJsonObject &obj)
{
    if (!isConnected()) {
        qWarning() << "[NetContext] not connected, drop data:"
                   << QJsonDocument(obj).toJson(QJsonDocument::Compact);
        return;
    }
    QByteArray payload = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    // 为了让服务端更易分帧，这里在每条 JSON 后加换行符（可根据实际协议删掉）
    payload.append('\n');
    client->write(payload);
    client->flush();
}

/* ------------------------------------------------------------------ */
/*   各种 push 接口 – 只负责把 type/value 包装成 JSON 再调用 sendJson   */
void NetContext::pushCo2(int value)
{
    QJsonObject obj;
    obj.insert("type", 10001);
    obj.insert("value", value);
    obj.insert("key", "value");
    sendJson(obj);
}
void NetContext::pushTe1(double value)
{
    QJsonObject obj;
    obj.insert("type", 10002);
    obj.insert("value", value);
    obj.insert("key", "value");
    sendJson(obj);
}
void NetContext::pushFire(bool value)
{
    QJsonObject obj;
    obj.insert("type", 10003);
    obj.insert("value", value);
    obj.insert("key", "value");
    sendJson(obj);
}
void NetContext::pushPM25(int value)
{
    QJsonObject obj;
    obj.insert("type", 10004);
    obj.insert("value", value);
    obj.insert("key", "value");
    sendJson(obj);
}
void NetContext::pushMethane(bool value)
{
    QJsonObject obj;
    obj.insert("type", 10005);
    obj.insert("value", value);
    obj.insert("key", "value");
    sendJson(obj);
}
void NetContext::pushLight(int value)
{
    QJsonObject obj;
    obj.insert("type", 10006);
    obj.insert("value", value);
    obj.insert("key", "value");
    sendJson(obj);
}
void NetContext::pushSmoke(bool value)
{
    QJsonObject obj;
    obj.insert("type", 10007);
    obj.insert("value", value);
    obj.insert("key", "value");
    sendJson(obj);
}
void NetContext::pushHumidity(double value)
{
    QJsonObject obj;
    obj.insert("type", 10008);
    obj.insert("value", value);
    obj.insert("key", "value");
    sendJson(obj);
}

void NetContext::readyReadSlot()
{
    // 追加新数据到持久化缓冲区，防止TCP分帧导致数据丢失
    m_buffer.append(client->readAll());
    handleData();
}

void NetContext::handleData()
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
        m_buffer.remove(0, frameEnd + 1);
        handleFrame(frame);
    }
}

void NetContext::handleFrame(const QByteArray &frame)
{
    QJsonObject rf = QJsonDocument::fromJson(frame).object();
    int type = rf.value("type").toInt();

    if(type == 20001){
        // 触发上报 CO2 数据
        SerialContext::getObject()->getCo2();
        qDebug() << "[NetContext] Trigger CO2 report";
    }
    else if(type == 20002){
        // 触发上报温度（te1）数据
        SerialContext::getObject()->getTemperature();
        qDebug() << "[NetContext] Trigger Temperature report";
    }
    else if(type == 20003){
        // 触发上报火光（fire）数据
        SerialContext::getObject()->getFire();
        qDebug() << "[NetContext] Trigger Fire report";
    }
    else if(type == 20004){
        // 触发上报 PM2.5 数据
        SerialContext::getObject()->getPM25();
        qDebug() << "[NetContext] Trigger PM2.5 report";
    }
    else if(type == 20005){
        // 触发上报甲烷数据
        SerialContext::getObject()->getMethane();
        qDebug() << "[NetContext] Trigger Methane report";
    }
    else if(type == 20006){
        // 触发上报光照数据
        SerialContext::getObject()->getLight();
        qDebug() << "[NetContext] Trigger Light report";
    }
    else if(type == 20007){
        // 触发上报烟雾数据
        SerialContext::getObject()->getSmoke();
        qDebug() << "[NetContext] Trigger Smoke report";
    }
    else if(type == 20008){
        // 触发上报湿度数据
        SerialContext::getObject()->getHumidity();
        qDebug() << "[NetContext] Trigger Humidity report";
    }

    if(type == 20011){
        int id = rf.value("id").toInt();
        int light = rf.value("light").toInt();
        /*if(id == 0)
            SerialContext::getObject()->setLED1(light);
        else */if(id == 1)
            SerialContext::getObject()
                ->setLED2(light);
    }
    else if(type == 20012){
        int r = rf.value("red").toInt();
        int g = rf.value("green").toInt();
        int b = rf.value("blue").toInt();
        SerialContext::getObject()
            ->setRGB(r,g,b);
    }
    else if(type == 20013){
        bool sw = rf.value("sw").toBool();
        SerialContext::getObject()
            ->setFun(sw);
    }
    else if(type == 20014){
        // 控制报警器
        bool sw = rf.value("sw").toBool();
        SerialContext::getObject()->setAlarm(sw);
    }
    else if(type == 20015){
        // 控制报警灯
        bool sw = rf.value("sw").toBool();
        SerialContext::getObject()->setAlarmLight(sw);
    }
    else {
        // 未知类型，可以打印警告日志
        qWarning() << "[NetContext] Unknown type:" << type;
    }
}

// ========== 自动重连实现 ==========

void NetContext::setAutoReconnect(bool enable)
{
    m_autoReconnect = enable;
    if (enable) {
        startReconnectTimer();
    } else {
        stopReconnectTimer();
    }
}

void NetContext::startReconnectTimer()
{
    if (m_reconnectTimerId == 0) {
        // 每5秒检查一次连接状态
        m_reconnectTimerId = startTimer(5000);
    }
}

void NetContext::stopReconnectTimer()
{
    if (m_reconnectTimerId != 0) {
        killTimer(m_reconnectTimerId);
        m_reconnectTimerId = 0;
    }
}

void NetContext::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_reconnectTimerId) {
        if (!m_autoReconnect || m_lastIp.isEmpty() || m_lastPort == 0)
            return;

        if (client->state() != QTcpSocket::ConnectedState) {
            tryReconnect();
        }
    }
}

void NetContext::tryReconnect()
{
    // 指数退避：5s → 10s → 20s → 30s（上限）
    int delay = qMin(BASE_RETRY_DELAY_MS * (1 << qMin(m_retryCount, 2)), MAX_RETRY_DELAY_MS);
    qDebug() << "[NetContext] 尝试重连" << m_lastIp << ":" << m_lastPort
             << "(第" << (m_retryCount + 1) << "次, 间隔" << delay / 1000 << "s)";

    client->connectToHost(QHostAddress(m_lastIp), m_lastPort);
    if (client->waitForConnected(3000)) {
        qDebug() << "[NetContext] 重连成功!";
        m_retryCount = 0;
        // 重新注册到服务器
        registerToServer(1);
    } else {
        m_retryCount++;
        qWarning() << "[NetContext] 重连失败:" << client->errorString();
    }
}






