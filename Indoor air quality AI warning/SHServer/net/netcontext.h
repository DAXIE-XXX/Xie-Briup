#ifndef NETCONTEXT_H
#define NETCONTEXT_H

#include <QObject>
class MyTcpServer;

/**
 * @brief 服务端网络管理类（单例）
 *        管理TCP服务器的启动和连接数限制
 */
class NetContext : public QObject
{
    Q_OBJECT
protected:
    explicit NetContext(QObject *parent = nullptr);

public:
    /** @brief 获取单例实例
     *  @return NetContext单例指针 */
    static NetContext *getObject();

    /** @brief 启动TCP服务器监听
     *  @param port 监听端口号
     *  @return 是否启动成功 */
    bool startServer(quint16 port);

    /** @brief 设置最大客户端连接数量
     *  @param limit 最大连接数 */
    void setClientLimit(int limit);

protected:
    static NetContext *obj;
    MyTcpServer *server;
};

#endif // NETCONTEXT_H