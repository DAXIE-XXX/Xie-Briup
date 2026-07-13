#include "netcontext.h"
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHostAddress>
#include <QTimerEvent>
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

void NetContext::sendLightColorChanged(int id, int roadId, int color)
{
    QJsonObject data;
    data.insert("type", 3001);  // 模式状态通知
    data.insert("light_id", id);       // 路口ID
    data.insert("road_id", roadId);
    data.insert("color", color);
    socket->write(QJsonDocument(data).toJson());
}

void NetContext::connectedSlot()
{
    sendType(1);
    if(reConId != -1)
        killTimer(reConId);
}

void NetContext::disconnectSlot()
{
    reConId = startTimer(5000);
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
    if(type == 6001){
        int id = rf.value("id").toInt();
        int mode = rf.value("mode").toInt();

        // 发送信号给界面控制路口
        emit crossingModeSet(id, mode);

        // 返回确认消息给服务器（使用正确的 id）
        sendModeStatus(id, mode);  // 传入正确的 id
    }
}

void NetContext::sendCrossingMode(int id, int mode)
{
    if(socket->state() != QTcpSocket::ConnectedState)
        return;

    QJsonObject data;
    data.insert("type", 6001);  // 路口模式设置消息类型
    data.insert("id", id);
    data.insert("mode", mode);
    socket->write(QJsonDocument(data).toJson());
}

void NetContext::sendModeStatus(int id, int mode)
{
    if(socket->state() != QTcpSocket::ConnectedState)
        return;

    QJsonObject data;
    data.insert("type", 4001);  // 模式状态通知
    data.insert("id", id);       // 路口ID
    data.insert("mode", mode);

    socket->write(QJsonDocument(data).toJson());
}
