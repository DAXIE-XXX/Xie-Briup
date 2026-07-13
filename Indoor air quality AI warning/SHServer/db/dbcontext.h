#ifndef DBCONTEXT_H
#define DBCONTEXT_H

#include <QObject>
class DBExec;
class QSemaphore;

/**
 * @brief 数据库连接池管理类（单例）
 *        维护多个DBExec实例，通过信号量控制并发访问
 */
class DBContext : public QObject
{
    Q_OBJECT
protected:
    explicit DBContext(QObject *parent = nullptr);

public:
    /** @brief 获取单例实例
     *  @return DBContext单例指针 */
    static DBContext *getObject();

    /** @brief 设置连接池参数（仅可调用一次）
     *  @param execLimit 连接池大小
     *  @param ip        数据库主机地址
     *  @param port      数据库端口
     *  @param username  数据库用户名
     *  @param password  数据库密码
     *  @param dbName    数据库名 */
    void setConfig(int execLimit, QString ip, quint16 port,
                   QString username, QString password,
                   QString dbName);

    /** @brief 初始化连接池，创建DBExec实例（必须在setConfig之后调用） */
    void initContext();

    /** @brief 检查连接池状态（是否所有连接都成功）
     *  @return 全部成功返回true */
    bool getState();

    /** @brief 获取连接池状态描述
     *  @return 状态描述字符串 */
    QString getStateMsg();

    /** @brief 从池中获取一个可用的DBExec（阻塞等待）
     *  @return 可用的DBExec指针，失败返回nullptr */
    DBExec *getDBExec();

    /** @brief 归还DBExec到池中
     *  @param exec 之前获取的DBExec指针 */
    void releaseDBExec(DBExec *exec);

protected:
    static DBContext *obj;
    QSemaphore *semaphore = nullptr;
    QVector<DBExec *> execList;
    int execLimit = -1;
    QString ip, username, password, dbName;
    quint16 port;
};

#endif // DBCONTEXT_H