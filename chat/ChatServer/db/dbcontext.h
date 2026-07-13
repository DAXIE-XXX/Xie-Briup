#ifndef DBCONTEXT_H
#define DBCONTEXT_H

#include <QObject>
class DBExec;
class QSemaphore;
class DBContext : public QObject
{
    Q_OBJECT
protected:
    explicit DBContext(QObject *parent = nullptr);

public:
    static DBContext *getObject();
    /**
     * @brief 设置连接池参数
     * @param execLimit 连接数量
     * @param ip
     * @param port
     * @param username
     * @param password
     * @param dbName
     */
    void setConfig(int execLimit, QString ip, quint16 port,
                   QString username,QString password,
                   QString dbName);
    /**
     * @brief 必须在setConfig之后调用，初始化连接池
     */
    void initContext();
    /**
     * @brief 获取当前的状态
     * @return
     */
    bool getState();
    QString getStateMsg();
    /**
     * @brief 申请操作对象
     * @return
     */
    DBExec *getDBExec();
    /**
     * @brief 释放操作对象
     * @param exec
     */
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




