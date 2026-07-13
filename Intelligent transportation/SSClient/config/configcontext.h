#ifndef CONFIGCONTEXT_H
#define CONFIGCONTEXT_H

#include <QObject>

class ConfigContext : public QObject
{
    Q_OBJECT
protected:
    explicit ConfigContext(QObject *parent = nullptr);

public:
    /**
     * @brief getServerListenPort获取监听的端口
     * @return
     */
    static int getServerListenPort();
    /**
     * @brief getServerMaxClient获取最大的客户端连接数量
     * @return
     */
    static int getServerMaxClient();

    // 新增：获取服务器信息
    static void getServerInfo(QString &ip, quint16 &port);

    // 新增：获取车辆信息
    static void getCarInfo(int &id, QString &Key);

    // 新增：获取车辆运行路线
    static void getCarRunList(QStringList &list);

signals:
};

#endif // CONFIGCONTEXT_H