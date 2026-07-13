#ifndef THREADSHANDLE_H
#define THREADSHANDLE_H

#include <QObject>

class ThreadsHandle : public QObject
{
    Q_OBJECT
public:
    explicit ThreadsHandle(QObject *parent = nullptr);
    /**
     * @brief 申请线程
     * @return
     */
    QThread *getTh();
    /**
     * @brief 回收线程
     * @param th
     */
    void releaseTh(QThread *th);

protected:
    QVector<QThread *> thList;
};

#endif // THREADSHANDLE_H







