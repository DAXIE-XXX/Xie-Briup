#include "netcontext.h"
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimerEvent>
#include <QDebug>
#include <QHostAddress>

NetContext *NetContext::obj = nullptr;

NetContext::NetContext(QObject *parent)
    : QObject{parent}
    , socket{new QTcpSocket(this)}
{
    connect(socket, &QTcpSocket::connected,
            this, &NetContext::connectedSlot);
    connect(socket, &QTcpSocket::disconnected,
            this, &NetContext::disconnectedSlot);
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
    data.insert("type", 1000);
    data.insert("client_type", type);
    socket->write(QJsonDocument(data).toJson());
}

void NetContext::sendCarRunPos(int id, int roadId, int roadPos)
{
    QJsonObject data;
    data.insert("type", 2008);
    data.insert("id", id);
    data.insert("road_id", roadId);
    data.insert("road_pos", roadPos);
    socket->write(QJsonDocument(data).toJson());
}

void NetContext::sendStateUpdate(const QJsonObject &data)
{
    if(socket && socket->state() == QTcpSocket::ConnectedState) {
        QJsonDocument doc(data);
        QByteArray jsonData = doc.toJson();
        socket->write(jsonData);
        qDebug() << "Sent state update:" << data;
    } else {
        qDebug() << "Cannot send state update - socket not connected";
    }
}

void NetContext::connectedSlot()
{
    sendType(0);
    if(reConId != -1)
        killTimer(reConId);
    qDebug() << "Car connected to server as type 0";
}

void NetContext::disconnectedSlot()
{
    qDebug() << "Car disconnected from server, reconnecting...";
    reConId = startTimer(5000);
}

void NetContext::readyReadSlot()
{
    QByteArray data = socket->readAll();
    handleData(data);
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

void NetContext::handleFrame(const QByteArray &frame)
{
    QJsonObject rf = QJsonDocument::fromJson(frame).object();
    int type = rf.value("type").toInt();

    qDebug() << "Car received message type:" << type;

    if(type == 5004){
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        QString key = rf.value("key").toString();
        emit carLockSet(id, sw, key);
    }
    else if(type == 5002){
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        QString key = rf.value("key").toString();
        emit carLightSet(id, sw, key);
    }
    else if(type == 5003){
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        QString key = rf.value("key").toString();
        emit carAlarmLightSet(id, sw, key);
    }
    else if(type == 5001){
        int id = rf.value("id").toInt();
        bool sw = rf.value("sw").toBool();
        QString key = rf.value("key").toString();
        emit carRunSet(id, sw, key);
    }
    // 添加处理红绿灯消息
    else if(type == 3001){
        int lightId = rf.value("light_id").toInt();
        int roadId = rf.value("road_id").toInt();
        int color = rf.value("color").toInt();
        qDebug() << "Car received light change - lightId:" << lightId
                 << "roadId:" << roadId << "color:" << color;
        emit lightColorChanged(lightId, roadId, color);
    }
}