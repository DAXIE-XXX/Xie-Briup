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

signals:
};

#endif // CONFIGCONTEXT_H
