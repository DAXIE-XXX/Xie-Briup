#include "netcontext.h"
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QTimerEvent>
#include <QDebug>
#include "datacontext.h"

NetContext *NetContext::obj = nullptr;

NetContext::NetContext(QObject *parent)
    : QObject{parent}
    , socket{new QTcpSocket(this)}
{
    connect(socket, &QTcpSocket::connected,
            this, &NetContext::connectedSlot);
    connect(socket, &QTcpSocket::disconnected,
            this, &NetContext::disconnectSlot);
    connect(socket, &QTcpSocket::readyRead,
            this, &NetContext::readyReadSlot);
}

NetContext *NetContext::getObject()
{
    if(obj == nullptr)
        obj = new NetContext;
    return obj;
}

void NetContext::connectToServer(QString ip, quint16 port)
{
    if(socket->state() == QTcpSocket::ConnectedState)
        socket->disconnectFromHost();
    socket->connectToHost(QHostAddress(ip), port);
    this->ip = ip;
    this->port = port;
    reConId = startTimer(10000);
}

void NetContext::sendType(int type)
{
    QJsonObject data;
    data.insert("type",1000);
    data.insert("client_type", type);
    socket->write(QJsonDocument(data).toJson());
}

void NetContext::sendCrossingMode(int id, int mode)
{
    QJsonObject data;
    data.insert("type",6001);
    data.insert("id", id);
    data.insert("mode", mode);
    socket->write(QJsonDocument(data).toJson());
}

void NetContext::sendCarLightSet(int id, bool sw)
{
    qDebug() << "NetContext::sendCarLightSet - id:" << id << "sw:" << sw;
    QJsonObject data;
    data.insert("type", 5002);
    data.insert("id", id);
    data.insert("sw", sw);
    data.insert("key", "ADMIN123");
    QByteArray jsonData = QJsonDocument(data).toJson();
    qDebug() << "Sending:" << jsonData;
    socket->write(jsonData);
}

void NetContext::sendCarAlarmSet(int id, bool sw)
{
    QJsonObject data;
    data.insert("type", 5003);
    data.insert("id", id);
    data.insert("sw", sw);
    data.insert("key", "ADMIN123");
    socket->write(QJsonDocument(data).toJson());
}

void NetContext::sendCarLockSet(int id, bool sw)
{
    QJsonObject data;
    data.insert("type", 5004);
    data.insert("id", id);
    data.insert("sw", sw);
    data.insert("key", "ADMIN123");
    socket->write(QJsonDocument(data).toJson());

    qDebug() << "Sent car light set - id:" << id << "sw:" << sw;
}

void NetContext::sendCarRunSet(int id, bool sw)
{
    QJsonObject data;
    data.insert("type", 5001);
    data.insert("id", id);
    data.insert("sw", sw);
    data.insert("key", "ADMIN123");
    socket->write(QJsonDocument(data).toJson());
    emit carRunSwChanged(id, sw);
}

void NetContext::connectedSlot()
{
    sendType(2);
    if(reConId != -1){
        killTimer(reConId);
        reConId = -1;
    }
    qDebug() << "Connected to server as type 2 (UI Client)";
    emit connectionStatusChanged(true); // 发射连接成功信号

}

void NetContext::disconnectSlot()
{
    qDebug() << "Disconnected from server, reconnecting...";
    if(reConId == -1)
        reConId = startTimer(5000);
    emit connectionStatusChanged(false);  // 发射断开连接信号
}

void NetContext::timerEvent(QTimerEvent *e)
{
    if(e->timerId() == reConId){
        socket->connectToHost(QHostAddress(ip), port);
    }
}

void NetContext::handleData(QByteArray &data)
{
    int count = 0;
    for(int i = 0; i < data.length(); i++){
        if(data.at(i) == '{')
            count++;
        else if(data.at(i) == '}'){
            count--;
            if(count == 0){
                QByteArray frame = data.mid(0, i+1);
                handleFrame(frame);
                if(frame.length() == data.length())
                    return;
                data = data.mid(i+1);
                i = -1;
            }
        }
    }
}

void NetContext::readyReadSlot()
{
    QByteArray data = socket->readAll();
    handleData(data);
}

void NetContext::handleFrame(const QByteArray &frame)
{
    QJsonObject rf = QJsonDocument::fromJson(frame).object();
    int type = rf.value("type").toInt();

    qDebug() << "Received message type:" << type << "data:" << rf;

    if(type == 4001){
        int id = rf.value("id").toInt();
        int mode = rf.value("mode").toInt();
        qDebug() << "Crossing" << id << "mode changed to:" << mode;
        emit modeStatusReceived(id, mode);
    }
    else if(type == 3001){
        int id = rf.value("light_id").toInt();
        int roadId = rf.value("road_id").toInt();
        int color = rf.value("color").toInt();
        qDebug() << "Light change - id:" << id << "roadId:" << roadId << "color:" << color;
        emit lightColorChanged(id, roadId, color);
    }
    else if(type == 2008) {
        int car_id   = rf.value("id").toInt();
        int road_id  = rf.value("road_id").toInt();
        int road_pos = rf.value("road_pos").toInt();
        DataContext::getObject()->setCarRunPos(car_id, road_id, road_pos);
    }
    else if(type == 2001){
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        emit carLightSwChanged(id, sw);
    }
    else if(type == 5003) {
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        emit carAlarmSwChanged(id, sw);
    }
    else if(type == 5002) {  // 车灯状态响应
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        qDebug() << "Received car light response - id:" << id << "sw:" << sw;
        emit carLightSwChanged(id, sw);
    }
    // 添加处理锁定状态
    else if(type == 5004) {  // 锁定状态响应
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        qDebug() << "Received car lock response - id:" << id << "sw:" << sw;
        emit carLockSwChanged(id, sw);
    }
    // 添加处理运行状态
    else if(type == 5001) {  // 运行状态响应
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        qDebug() << "Received car run response - id:" << id << "sw:" << sw;
        emit carRunSwChanged(id, sw);
    }
    // 在处理类型中添加
    else if(type == 2009) {  // 续航和里程数据
        int car_id = rf.value("id").toInt();
        double endurance = rf.value("endurance").toDouble();
        double mileage = rf.value("mileage").toDouble();
        qDebug() << "Received endurance/mileage - car:" << car_id
                 << "endurance:" << endurance
                 << "mileage:" << mileage;
        DataContext::getObject()->setCarEnduranceMileage(car_id, endurance, mileage);
    }
}