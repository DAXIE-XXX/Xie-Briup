#ifndef DATACONTEXT_H
#define DATACONTEXT_H

#include <QObject>
#include <QJsonArray>
#include <QTimer>

// 前向声明
class NetContext;

/**
 * @brief 道路信息类
 */
class RoadInfo {
    friend class DataContext;
public:
    RoadInfo(int x, int y, int w, int h, int dir);
protected:
    int x, y, w, h;
    int color = 0;  //0-g 1-y 2-r
    int dir = -1;   //0-left 1-right 2-up 3-down
};

/**
 * @brief 红绿灯信息类
 */
class LightInfo {
    friend class DataContext;
public:
    LightInfo(int roadId);
protected:
    int roadId;     //道路id
    int color = 0;  //0-green 1-yellow 2-red -1-off
};

class CarInfo{
    friend class DataContext;
public:
    CarInfo();
protected:
    int roadId = -1;
    int roadPos = -1;
    bool isRunning = false;   // 车辆运行状态
    bool isLightOn = false;   // 车灯状态
    bool isAlarmOn = false;    // 双闪状态
    bool isLocked = false;    // 锁定状态
    bool isWaitingAtRedLight = false;  // 是否在红灯前等待
    int waitingLightId = -1;  // 等待的红绿灯ID
    double endurance = 100.0; // 续航里程 (KM)
    double mileage = 0.0;     // 已行驶里程 (KM)
};

class DataContext : public QObject
{
    Q_OBJECT
protected:
    explicit DataContext(QObject *parent = nullptr);

public:
    static DataContext *getObject();

    void setCarEnduranceMileage(int id, double endurance, double mileage);

    Q_INVOKABLE QJsonArray getRoadList();   //道路列表
    Q_INVOKABLE QJsonArray getLightList();  //路灯列表
    Q_INVOKABLE QJsonArray getCarList();    //车列表

    /**
     * @brief 修改信号灯颜色
     * @param id 信号的id
     * @param color 信号灯颜色 (0-green, 1-yellow, 2-red, -1-off)
     */
    Q_INVOKABLE void setLightColor(int id, int color);

    // 车辆状态获取
    Q_INVOKABLE bool getCarRunning(int id);
    Q_INVOKABLE bool getCarLight(int id);
    Q_INVOKABLE bool getCarAlarm(int id);
    Q_INVOKABLE bool getCarLock(int id);

    // 车辆控制（会发送网络消息）
    Q_INVOKABLE void setCarRunning(int id, bool running);
    Q_INVOKABLE void setCarLight(int id, bool light);
    Q_INVOKABLE void setCarAlarm(int id, bool alarm);
    Q_INVOKABLE void setCarLock(int id, bool locked);

    // 页面调用接口
    Q_INVOKABLE void pageSetCarAlarm(int carId, bool sw);
    Q_INVOKABLE void pageSetCarLight(int carId, bool sw);
    Q_INVOKABLE void pageSetCarLock(int carId, bool sw);
    Q_INVOKABLE void pageSetCarRun(int carId, bool sw);

    // **新增** —— 直接在本地启动/停止车辆（不再依赖服务器回包）
    Q_INVOKABLE void startCarLocally(int id);
    Q_INVOKABLE void stopCarLocally(int id);

    // 车辆位置
    Q_INVOKABLE int getCarPosX(int carId);
    Q_INVOKABLE int getCarPosY(int carId);

    // 移动控制
    Q_INVOKABLE void startCarMovement(int carId);
    Q_INVOKABLE void stopCarMovement(int carId);

    Q_INVOKABLE double getCarEndurance(int id);
    Q_INVOKABLE double getCarMileage(int id);
    Q_INVOKABLE QString getCarWarning(int id); // 新增：获取警告信息

    /**
     * @brief setCarRunPos修改车辆位置
     * @param id
     * @param roadId
     * @param roadPos
     */
    void setCarRunPos(int id, int roadId, int roadPos);

signals:
    void dataChanged();
    void carLightSwChanged();
    void carAlarmSwChanged();

protected slots:
    void onCarLightSwChanged(int id, bool sw);
    void onCarAlarmSwChanged(int id, bool sw);
    void onCarRunSwChanged(int id, bool sw);
    void onCarLockSwChanged(int id, bool sw);
    void updateCarPositions();
    void onLightColorChanged(int id, int roadId, int color);

protected:
    void autoSetRoadColor(int roadId);
    bool isCarBlockedByRedLight(int carId);
    int getLightIdAtCarPosition(int carId);
    static DataContext *obj;
    QVector<RoadInfo *> rl;   //道路信息
    QVector<LightInfo *> ll;  //路灯信息
    QVector<CarInfo *> cl;    //车信息

    NetContext *nc = nullptr;

    QTimer *movementTimer;
    QVector<bool> carMoving;
};

#endif // DATACONTEXT_H