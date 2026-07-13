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
     * @brief 获取监听的端口
     * @return
     */
    static int getServerListenPort();
    /**
     * @brief 获取最大的客户端连接数量
     * @return
     */
    static int getServerMaxClient();
    /**
     * @brief 设置配置文件路径
     * @param path
     */
    static void setConfigFilePath(QString path);
    /**
     * @brief 获取连接池的数量限制
     * @return
     */
    static int getDBLimit();
    /**
     * @brief 获取数据库配置
     * @param ip
     * @param port
     * @param username
     * @param password
     * @param dbName
     */
    static void getDBConfig(QString &ip, quint16 &port,
                            QString &username, QString &password,
                            QString &dbName);


protected:
    static QString CONF_FILENAME;
};

#endif // CONFIGCONTEXT_H






