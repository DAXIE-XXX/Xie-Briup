#ifndef LOGCONTEXT_H
#define LOGCONTEXT_H

#include <QObject>

class LogContext : public QObject
{
    Q_OBJECT
protected:
    explicit LogContext(QObject *parent = nullptr);

public:
    static LogContext *getObject();
    /**
     * @brief 输出日志
     * @param msg
     */
    void showLog(QString msg);
    void showCritical(QString msg);
    void showWarning(QString msg);

protected:
    static LogContext *obj;

};

#endif // LOGCONTEXT_H




