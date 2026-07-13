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
        .arg(QDateTime::currentDateTime()
            .toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(msg);
    qInfo() << data;
}

void LogContext::showCritical(QString msg)
{
    QString data = QString("[%1](Error):%2")
    .arg(QDateTime::currentDateTime()
             .toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(msg);
    qCritical() << data;
}

void LogContext::showWarning(QString msg)
{
    QString data = QString("[%1](Warning):%2")
    .arg(QDateTime::currentDateTime()
             .toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(msg);
    qWarning() << data;
}






