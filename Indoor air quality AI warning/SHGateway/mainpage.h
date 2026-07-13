#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <QWidget>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainPage; }
QT_END_NAMESPACE

class SerialContext;          // 前向声明

class MainPage : public QWidget
{
    Q_OBJECT
public:
    explicit MainPage(QWidget *parent = nullptr);
    ~MainPage() override;

private slots:
    /* 串口相关 */
    void onOpenSerialClicked();
    void onCloseSerialClicked();

    /* 设备控制 */
    void onAlarmOnClicked();
    void onAlarmOffClicked();
    void onAlarmLightOnClicked();
    void onAlarmLightOffClicked();
    void onLED2OnClicked();
    void onLED2OffClicked();
    void onFanOnClicked();
    void onFanOffClicked();

    /* 传感器检测 */
    void onPM25Clicked();
    void onSmokeClicked();
    void onMethaneClicked();
    void onLightClicked();
    void onCo2Clicked();
    void onFireClicked();
    void onTemperatureClicked();
    void onHumidityClicked();

    /* 传感器数据更新 */
    void updatePM25(int value);
    void updateSmoke(bool value);
    void updateMethane(bool value);
    void updateLight(int value);
    void updateCo2(int value);
    void updateFire(bool value);
    void updateTemperature(double value);
    void updateHumidity(double value);

    // 服务器连接
    void onConnectServerClicked();

private:
    void timerEvent(QTimerEvent *e);
    Ui::MainPage *ui;
    SerialContext *serial;          // 单例，避免每次 getObject() 都写长串

    void fillPortList();           // 把系统可用端口填入 comboPort
    void fillBaudRateList();       // 填入常用波特率
    bool tryOpenSerial();          // 公共的打开检查，返回 true 表示已打开
    /** 检查是否已经连接到服务器。如果未连接弹出一次警告并返回 false。 */
    bool warnIfNotConnected();
};

#endif // MAINPAGE_H