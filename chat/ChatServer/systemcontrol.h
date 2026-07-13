#ifndef SYSTEMCONTROL_H
#define SYSTEMCONTROL_H

#include <QObject>

class SystemControl : public QObject
{
    Q_OBJECT
public:
    explicit SystemControl(QObject *parent = nullptr);
    /**
     * @brief 系统初始化
     */
    void systemInit(QString path);
    /**
     * @brief 系统启动
     */
    void systemStart();

protected slots:
    void mqttConnectedSlot();

protected:
    void timerEvent(QTimerEvent *e);
    int mqttCheck = -1;

};

#endif // SYSTEMCONTROL_H
