#ifndef SYSTEMCONTROL_H
#define SYSTEMCONTROL_H

#include <QObject>

/**
 * @brief 系统控制类
 *        负责系统初始化（配置/数据库/MQTT）和启动（TCP监听）
 */
class SystemControl : public QObject
{
    Q_OBJECT
public:
    explicit SystemControl(QObject *parent = nullptr);

    /** @brief 系统初始化：加载配置、初始化数据库连接池、初始化MQTT连接池
     *  @param path 配置文件路径 */
    void systemInit(QString path);

    /** @brief 系统启动：启动TCP服务器监听 */
    void systemStart();

protected:
};

#endif // SYSTEMCONTROL_H