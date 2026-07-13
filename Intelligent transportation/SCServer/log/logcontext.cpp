#include "logcontext.h"
#include <QDateTime>
LogContext *LogContext::obj = nullptr;
LogContext::LogContext(QObject *parent)
    : QObject{parent}
{}

LogContext *LogContext::getObject()
{
    if(obj == nullptr)
        obj = new LogContext;
    return obj;
}

void LogContext::showLog(QString msg)
{
    QString data = QString("[%1]:%2")
    .arg(QDateTime::currentDateTime(). toString("yyyy-mm-dd hh:mm:ss.zzz")) .arg(msg);
    qInfo() << data;
}