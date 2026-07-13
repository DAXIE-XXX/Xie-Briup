#ifndef SYSTEMCONTROL_H
#define SYSTEMCONTROL_H

#include <QObject>
#include <QHash>
#include <QStringList>

class SystemControl : public QObject
{
    Q_OBJECT
protected:
    explicit SystemControl(QObject *parent = nullptr);

public:
    static SystemControl *getObject();
    void systemInit();
    void systemStart();
    /**
     * @brief 获取车辆id
     * @return
     */
    Q_INVOKABLE int getCarId();
    /**
     * @brief 设置车辆锁定状态
     * @param sw
     */
    Q_INVOKABLE void setCarLock(bool sw);
    /**
     * @brief 设置车辆运行状态
     * @param sw
     */
    Q_INVOKABLE void setCarRun(bool sw);

    /**
     * @brief 获取红绿灯颜色
     * @param roadId 道路ID
     * @return 0=绿灯, 1=黄灯, 2=红灯
     */
    Q_INVOKABLE int getLightColor(int roadId);

signals:
    void lockStateChanged(bool sw);
    void runStateChanged(bool sw);

protected slots:
    void carLockSetSlot(int id, bool sw, QString key);
    void carLightSetSlot(int id, bool sw, QString key);
    void carAlarmLightSetSlot(int id, bool sw, QString key);
    void carRunSetSlot(int id, bool sw, QString key);
    void onLightColorChanged(int lightId, int roadId, int color);

protected:
    void timerEvent(QTimerEvent *e);
    static SystemControl *obj;
    int carId = -1;
    QString carKey;
    QStringList runList;
    int runRoadIndex = 0;
    int runRoadPos = 0;

    bool lockState = true;  //默认锁定
    bool runState = false;  //默认停止

    // 存储红绿灯状态 (roadId -> color)
    // 颜色定义: 0=绿灯, 1=黄灯, 2=红灯
    QHash<int, int> lightStates;

    // 检查是否被红灯阻挡
    bool isBlockedByRedLight(int currentRoadId, int currentRoadPos);
};

#endif // SYSTEMCONTROL_H