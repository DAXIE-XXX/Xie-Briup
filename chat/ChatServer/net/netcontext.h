#ifndef NETCONTEXT_H
#define NETCONTEXT_H

#include <QObject>
class MyTcpServer;
class NetContext : public QObject
{
    Q_OBJECT
protected:
    explicit NetContext(QObject *parent = nullptr);

public:
    static NetContext *getObject();
    /**
     * @brief 启动服务器
     * @param port
     * @return
     */
    bool startServer(quint16 port);
    /**
     * @brief 修改最大连接数量
     * @param limit
     */
    void setClientLimit(int limit);

protected:
    static NetContext *obj;
    MyTcpServer *server;
};

#endif // NETCONTEXT_H







