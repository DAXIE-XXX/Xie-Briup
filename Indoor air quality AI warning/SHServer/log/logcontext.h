#ifndef LOGCONTEXT_H
#define LOGCONTEXT_H

#include <QObject>

/**
 * @brief 日志管理类（单例）
 *        将日志输出到控制台和日志文件
 */
class LogContext : public QObject
{
    Q_OBJECT
protected:
    explicit LogContext(QObject *parent = nullptr);

public:
    /** @brief 获取单例实例
     *  @return LogContext单例指针 */
    static LogContext *getObject();

    /** @brief 输出普通日志
     *  @param msg 日志内容 */
    void showLog(QString msg);

    /** @brief 输出严重错误日志
     *  @param msg 日志内容 */
    void showCritical(QString msg);

    /** @brief 输出警告日志
     *  @param msg 日志内容 */
    void showWarning(QString msg);

protected:
    static LogContext *obj;
};

#endif // LOGCONTEXT_H
