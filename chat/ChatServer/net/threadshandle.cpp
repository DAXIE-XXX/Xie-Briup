#include "threadshandle.h"
#include <QThread>
#include <QVariant>
#define MAXTHLIMIT 500

ThreadsHandle::ThreadsHandle(QObject *parent)
    : QObject{parent}
{
    for(int i = 0; i < MAXTHLIMIT; i++){
        QThread *th = new QThread(this);
        th->start();
        thList.append(th);
    }
}

QThread *ThreadsHandle::getTh()
{//OBJ_COUNT
    int index = 0;
    int count = thList.at(0)
                    ->property("OBJ_COUNT")
                    .toInt();
    for(int i = 1; i < MAXTHLIMIT; i++){
        int temp = thList.at(i)
                       ->property("OBJ_COUNT")
                       .toInt();
        if(temp < count){
            index = i;
            count = temp;
        }
    }
    QThread *th = thList.at(index);
    th->setProperty("OBJ_COUNT", count+1);
    return th;
}

void ThreadsHandle::releaseTh(QThread *th)
{
    if(th == nullptr)
        return;
    int count = th->property("OBJ_COUNT").toInt();
    th->setProperty("OBJ_COUNT", count-1);
}






