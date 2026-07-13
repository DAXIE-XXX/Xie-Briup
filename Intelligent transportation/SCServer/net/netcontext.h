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
     * @brief startServer启动服务器
     * @param port
     * @return
     */
    bool startServer(quint16 port);
    /**
     * @brief setClientLimit修改最大连接数
     * @param limit
     */
    void setClientLimit(int limit);

signals:
    void newClientConnected(int type);
    void clientDisconnected(int type);

protected:
    MyTcpServer *server;
    static NetContext *obj;
};

#endif // NETCONTEXT_H
