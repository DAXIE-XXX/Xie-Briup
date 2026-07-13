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
    void systemInit();
    /**
     * @brief 系统启动
     */
    void systemStart();

protected slots:
    void newClientConnectedSlot(int type);
    void clientDisconnectedSlot(int type);
};

#endif // SYSTEMCONTROL_H
