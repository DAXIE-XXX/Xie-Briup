#ifndef CONFIGCONTEXT_H
#define CONFIGCONTEXT_H

#include <QObject>

/**
 * @brief 服务端配置管理类
 *        从INI配置文件读取服务器、数据库、MQTT等配置参数
 */
class ConfigContext : public QObject
{
    Q_OBJECT
protected:
    explicit ConfigContext(QObject *parent = nullptr);

public:
    /** @brief 获取TCP服务器监听端口
     *  @return 端口号，默认10086 */
    static int getServerListenPort();

    /** @brief 获取最大客户端连接数量
     *  @return 最大连接数，默认500 */
    static int getServerMaxClient();

    /** @brief 设置配置文件路径
     *  @param path INI配置文件的绝对路径 */
    static void setConfigFilePath(QString path);

    /** @brief 获取数据库连接池大小
     *  @return 连接数，默认50 */
    static int getDBLimit();

    /** @brief 获取数据库连接配置
     *  @param ip       [输出] 主机地址
     *  @param port     [输出] 端口
     *  @param username [输出] 用户名
     *  @param password [输出] 密码
     *  @param dbName   [输出] 数据库名 */
    static void getDBConfig(QString &ip, quint16 &port,
                            QString &username, QString &password,
                            QString &dbName);

protected:
    static QString CONF_FILENAME;
};

#endif // CONFIGCONTEXT_H
