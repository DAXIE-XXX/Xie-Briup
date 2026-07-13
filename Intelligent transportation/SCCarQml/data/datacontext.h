#ifndef DATACONTEXT_H
#define DATACONTEXT_H

#include <QObject>
#include <QJsonArray>
#include <QHash>  // 添加这个头文件

class CarLight;
class CarAlarmLight;

class DataContext : public QObject
{
    Q_OBJECT

public:  // 将函数移到 public 区域
    explicit DataContext(QObject *parent = nullptr);
    static DataContext *getObject();

    Q_INVOKABLE void setCarLightSw(bool sw);
    Q_INVOKABLE void setCarAlarmSw(bool sw);
    Q_INVOKABLE bool getCarLightSw();
    Q_INVOKABLE bool getCarAlarmSw();
    Q_INVOKABLE QJsonArray getCarLightList();
    Q_INVOKABLE QJsonArray getCarAlarmLightList();

    // 这些函数需要是 public 的，因为 systemcontrol.cpp 会调用它们
    double getCarEndurance();
    double getCarMileage();

    void setCrossingLight(int roadId, int color);
    int getCrossingLight(int roadId);
    void updateCarMileage(double delta);
    void updateCarEndurance(double delta);

signals:
    void carLightSwChanged();
    void carAlarmSwChanged();
    void dataChanged();

protected:
    static DataContext *obj;
    bool carLightSw = false;
    bool carAlarmSw = false;
    QVector<CarLight *> cll;
    QVector<CarAlarmLight *> call;
    QHash<int, int> crossingLightList;

    double endurance = 100.0;  // 续航里程
    double mileage = 0.0;      // 已行驶里程
};

class CarLight {
    friend class DataContext;
public:
    CarLight(int x, int y, int w, int h);
protected:
    int x,y,w,h;
};

class CarAlarmLight {
    friend class DataContext;
public:
    CarAlarmLight(int x, int y, int w, int h);
protected:
    int x,y,w,h;
};

#endif // DATACONTEXT_H