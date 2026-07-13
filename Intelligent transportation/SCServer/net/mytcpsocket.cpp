#include "mytcpsocket.h"
#include <QJsonObject>
#include <QJsonDocument>

MyTcpSocket::MyTcpSocket(QObject *parent)
    : QTcpSocket{parent}
{
    connect(this, &MyTcpSocket::readyRead, this, &MyTcpSocket::readyReadSlot);
}

void MyTcpSocket::serverForwardToClientSlot(int clientType, const QByteArray &data)
{
    if(ct != clientType || ct < 0)
        return;
    write(data);
}

void MyTcpSocket::readyReadSlot()
{
    QByteArray data = this->readAll();
    handleData(data);
}

void MyTcpSocket::handleData(QByteArray &data)
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

void MyTcpSocket::handleFrame(const QByteArray &frame)
{
    QJsonObject rf = QJsonDocument::fromJson(frame).object();
    int type = rf.value("type").toInt();
    if(type == 1000){
        ct = rf.value("client_type").toInt();
        if(ct != -1)
            emit newClientConnected(ct);
    }
    // 转发模式状态通知（type=4001）给客户端（类型2）
    else if(type == 4001){
        emit forwardToClient(2, frame);
    }
    // 2001-2008：向类型2和3的客户端广播
    else if(type >= 2001 && type <= 2008){
        emit forwardToClient(2, frame);
        emit forwardToClient(3, frame);
    }
    // 3001：向类型0、2、3的客户端广播
    else if(type == 3001){
        emit forwardToClient(0, frame);
        emit forwardToClient(2, frame);
        emit forwardToClient(3, frame);
    }
    // 4001：向类型2和3的客户端广播
    else if(type == 4001){
        emit forwardToClient(2, frame);
        emit forwardToClient(3, frame);
    }
    // 5001-5004：只向类型0的客户端发送
    else if(type >= 5001 && type <= 5004){
        emit forwardToClient(0, frame);
    }
    // 6001：处理路口模式设置请求
    else if(type == 6001){
        // 1. 转发给模拟器客户端（类型1）- 实际控制路口
        emit forwardToClient(1, frame);

        // 2. 获取请求中的id和mode
        int id = rf.value("id").toInt();
        int mode = rf.value("mode").toInt();

        // 3. 构造响应消息返回给UI客户端
        QJsonObject response;
        response.insert("type", 4001);
        response.insert("id", id);
        response.insert("mode", mode);
        QByteArray responseFrame = QJsonDocument(response).toJson();

        // 4. 发送响应给UI客户端（类型2）和可能的其他UI客户端
        emit forwardToClient(2, responseFrame);
        // 可选：也发给类型3（如果有其他监控端）
        // emit forwardToClient(3, responseFrame);
    }
}