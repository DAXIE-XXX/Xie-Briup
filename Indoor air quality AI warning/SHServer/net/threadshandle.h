#ifndef THREADSHANDLE_H
#define THREADSHANDLE_H

#include <QObject>

/**
 * @brief 线程池管理类
 *        预创建500个工作线程，采用负载均衡分配给TCP连接
 */
class ThreadsHandle : public QObject
{
    Q_OBJECT
public:
    explicit ThreadsHandle(QObject *parent = nullptr);

    /** @brief 获取一个负载最轻的线程
     *  @return 可用的QThread指针 */
    QThread *getTh();

    /** @brief 归还线程（减少引用计数）
     *  @param th 之前获取的QThread指针 */
    void releaseTh(QThread *th);

protected:
    QVector<QThread *> thList;
};

#endif // THREADSHANDLE_H