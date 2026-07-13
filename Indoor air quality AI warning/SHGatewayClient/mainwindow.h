#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QChart>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QSpinBox;  // 前向声明，mainwindow.cpp 中包含完整头文件

struct SensorData {
    double temperature = 0.0;
    double humidity = 0.0;
    double co2 = 0.0;
    double pm25 = 0.0;
    double methane = 0.0;
    double light = 0.0;
    double smoke = 0.0;
    double fire = 0.0;
};

enum SensorType {
    SENSOR_TEMPERATURE = 10002,
    SENSOR_HUMIDITY = 10008,
    SENSOR_CO2 = 10001,
    SENSOR_PM25 = 10004,
    SENSOR_METHANE = 10005,
    SENSOR_LIGHT = 10006,
    SENSOR_SMOKE = 10007,
    SENSOR_FIRE = 10003
};

struct SensorInfo {
    SensorType type;
    QString name;
    QString tableName;
    QString unit;
    QString displayFormat;
};

// ========== 阈值配置结构体 ==========
struct ThresholdConfig {
    int roomId = 1;
    int co2Max = 1000;
    int pm25Max = 50;
    double tempMax = 30.0;
    double tempMin = 18.0;
    bool methaneEnable = true;
    int cooldownSeconds = 30;       // 告警冷却（秒），避免同一传感器频繁触发
    int fanCooldownSeconds = 30;    // 风扇关闭延迟（秒），数值正常后再运行N秒关闭
};

class TcpClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setServerInfo(const QString &ip, quint16 port);
    void connectToServer();

    // ========== 新增：公开方法 ==========
    void sendDeviceRegister();
    void sendThresholdSetting();
    void loadDefaultThresholds();

private slots:
    void onConnected();
    void onDisconnected();
    void onError(const QString &error);
    void onDataReceived(const QJsonObject &data);
    void onServerCommandReceived(const QJsonObject &cmd);  // 新增

    void onLED2SetClicked();
    void onRGBSetClicked();
    void onFanOnClicked();
    void onFanOffClicked();
    void onAlarmOnClicked();
    void onAlarmOffClicked();
    void onAlarmLightOnClicked();
    void onAlarmLightOffClicked();

    void onCo2Clicked();
    void onTemperatureClicked();
    void onFireClicked();
    void onPM25Clicked();
    void onMethaneClicked();
    void onLightClicked();
    void onSmokeClicked();
    void onHumidityClicked();

    void requestNextSensor();
    void onHistoryButtonClicked();
    void onShowHistoryClicked();
    void onShowWeekDataClicked();     // 查看一周内全部数据
    void onShowWeekAvgClicked();      // 查看一周内每日均值
    void onShowMonthDataClicked();    // 查看一月内全部数据
    void onShowMonthAvgClicked();     // 查看一月内每日均值

    // ========== 新增：阈值设置相关槽函数 ==========
    void onApplyThresholdClicked();
    void onLoadDefaultClicked();

private:
    Ui::MainWindow *ui;
    TcpClient *tcpClient;
    SensorData sensorData;

    QString serverIP;
    quint16 serverPort;

    QTimer *autoRequestTimer;
    int currentSensorIndex;
    QList<int> sensorTypes;

    QSqlDatabase db;
    bool dbConnected;

    QList<SensorInfo> sensorInfoList;

    // 历史数据窗口
    QWidget *historyWindow;
    QChartView *chartView;
    QLineSeries *series;
    QDateTimeAxis *axisX;
    QValueAxis *axisY;
    SensorType currentSensorType = SENSOR_TEMPERATURE;  // 当前查看的传感器类型

    // 全部数据窗口
    QWidget *allDataWindow;
    QList<QChartView*> allChartViews;
    QList<QLineSeries*> allSeries;
    QList<QDateTimeAxis*> allAxisX;
    QList<QValueAxis*> allAxisY;

    // ========== 新增：阈值配置 ==========
    ThresholdConfig thresholdConfig;
    QSpinBox *spinFanCooldown = nullptr;  // 风扇关闭延迟输入框（动态创建）

    void updateConnectionStatus(bool connected);
    void sendCommand(const QJsonObject &cmd);
    void updateSensorDisplay();
    void initSensorLabels();
    void startAutoRequest();
    void stopAutoRequest();

    bool initDatabase();
    void querySensorHistory(SensorType type);
    void queryWeekHistory(SensorType type);       // 查询一周内全部数据
    void queryWeekAvgHistory(SensorType type);    // 查询一周内每日均值
    void queryMonthHistory(SensorType type);      // 查询一月内全部数据
    void queryMonthAvgHistory(SensorType type);   // 查询一月内每日均值
    void updateChart(const QString &title, const QString &unit);

    void createHistoryWindow();
    void showHistoryForSensor(int sensorIndex);

    void createAllDataWindow();
    void queryAllSensorHistory();
    void onShowAllDataClicked();

    // ========== 新增：UI辅助函数 ==========
    void updateThresholdUI();
    void updateThresholdStatus(const QString &status, bool success);
};

#endif // MAINWINDOW_H