#include "netcontext.h"
#include "mytcpserver.h"
NetContext *NetContext::obj = nullptr;
NetContext::NetContext(QObject *parent)
    : QObject{parent}
    , server{new MyTcpServer(this)}
{
    connect(server, &MyTcpServer::newClientConnected, this, &NetContext::newClientConnected);
    connect(server, &MyTcpServer::clientDisconnected, this, &NetContext::clientDisconnected);
}

bool NetContext::startServer(quint16 port)
{
    if(server->isListening())
        return false;
    return server->listen(QHostAddress::Any, port);
}

void NetContext::setClientLimit(int limit)
{
    server->limit = limit;
}


NetContext *NetContext::getObject()
{
    if(obj == nullptr)
        obj = new NetContext;
    return obj;
}

