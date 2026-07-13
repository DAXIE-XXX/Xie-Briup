#ifndef SENSORCONTROLLER_H
#define SENSORCONTROLLER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QTimer>

class TcpClient;

/**
 * @brief 传感器业务逻辑控制器
 *        将核心业务逻辑从 mainwindow 提取为独立 QObject，
 *        通过 Q_PROPERTY 和 Q_INVOKABLE 暴露给 QML 界面层。
 *
 * 职责：
 *  - TCP 连接管理
 *  - 传感器数据存取与通知
 *  - 设备控制指令发送
 *  - 阈值配置管理
 *  - 历史数据查询
 */
class SensorController : public QObject
{
    Q_OBJECT

    // ---- QML 可读属性 ----
    Q_PROPERTY(bool     connected       READ isConnected      NOTIFY connectedChanged)
    Q_PROPERTY(QString  serverIp        READ serverIp         NOTIFY serverIpChanged)
    Q_PROPERTY(int      serverPort      READ serverPort       NOTIFY serverPortChanged)

    // 传感器数据（通过 dataUpdated 信号批量通知）
    Q_PROPERTY(double   temperature     READ temperature      NOTIFY dataChanged)
    Q_PROPERTY(double   humidity        READ humidity         NOTIFY dataChanged)
    Q_PROPERTY(double   co2             READ co2              NOTIFY dataChanged)
    Q_PROPERTY(double   pm25            READ pm25             NOTIFY dataChanged)
    Q_PROPERTY(bool     methane         READ methane          NOTIFY dataChanged)
    Q_PROPERTY(double   light           READ light            NOTIFY dataChanged)
    Q_PROPERTY(bool     smoke           READ smoke            NOTIFY dataChanged)
    Q_PROPERTY(bool     fire            READ fire             NOTIFY dataChanged)

    // 阈值配置
    Q_PROPERTY(int      co2Max          READ co2Max           NOTIFY thresholdChanged)
    Q_PROPERTY(int      pm25Max         READ pm25Max          NOTIFY thresholdChanged)
    Q_PROPERTY(double   tempMax         READ tempMax          NOTIFY thresholdChanged)
    Q_PROPERTY(double   tempMin         READ tempMin          NOTIFY thresholdChanged)
    Q_PROPERTY(bool     methaneEnable   READ methaneEnable    NOTIFY thresholdChanged)
    Q_PROPERTY(int      cooldownSeconds READ cooldownSeconds  NOTIFY thresholdChanged)
    Q_PROPERTY(int      fanCooldownSeconds READ fanCooldownSeconds NOTIFY thresholdChanged)

public:
    explicit SensorController(QObject *parent = nullptr);
    ~SensorController() override;

    // ---- 基本属性访问 ----
    bool isConnected() const;
    QString serverIp() const;
    int serverPort() const;

    // ---- 传感器数据访问 ----
    double temperature() const;
    double humidity() const;
    double co2() const;
    double pm25() const;
    bool   methane() const;
    double light() const;
    bool   smoke() const;
    bool   fire() const;

    // ---- 阈值访问 ----
    int    co2Max() const;
    int    pm25Max() const;
    double tempMax() const;
    double tempMin() const;
    bool   methaneEnable() const;
    int    cooldownSeconds() const;
    int    fanCooldownSeconds() const;

public slots:
    // ---- QML 可调用方法 ----

    /** 连接服务器 */
    Q_INVOKABLE void connectToServer(const QString &ip, int port);

    /** 断开连接 */
    Q_INVOKABLE void disconnectFromServer();

    /** 请求单个传感器数据 (type: 20001~20008) */
    Q_INVOKABLE void requestSensor(int sensorType);

    /** 发送设备控制指令 */
    Q_INVOKABLE void sendControl(int controlType, const QVariantMap &params);

    /** 下发阈值配置给服务器 */
    Q_INVOKABLE void applyThreshold(int co2Max, int pm25Max, double tempMax,
                                     double tempMin, bool methaneEnable,
                                     int cooldownSeconds, int fanCooldownSeconds);

    /** 查询历史数据 (sensorType: 10001~10008, range: "1h"/"24h"/"7d"/"30d") */
    Q_INVOKABLE void queryHistory(int sensorType, const QString &range);

signals:
    /** 连接状态变化 */
    void connectedChanged();
    void serverIpChanged();
    void serverPortChanged();

    /** 传感器数据已更新（QML 重新读取所有属性） */
    void dataChanged();

    /** 阈值配置已更新 */
    void thresholdChanged();

    /** 告警通知 (type 20050) */
    void alertReceived(const QString &action, double value, const QString &timestamp);

    /** 历史数据查询结果（JSON 字符串，QML 端用 JSON.parse 解析） */
    void historyReceived(int sensorType, const QString &range, const QString &dataJson);

    /** 历史数据查询失败 */
    void historyQueryError(const QString &msg);

    /** 连接错误 */
    void connectionError(const QString &errorMsg);

    /** 阈值设置确认 */
    void thresholdApplied(bool success);

private slots:
    /** 接收 TCP 数据 */
    void onDataReceived(const QJsonObject &data);

    /** 接收服务器命令（告警） */
    void onServerCommand(const QJsonObject &cmd);

    /** TCP 连接成功 */
    void onConnected();

    /** TCP 断开 */
    void onDisconnected();

    /** TCP 错误 */
    void onError(const QString &error);

private:
    TcpClient *m_tcpClient;

    // 传感器数据
    double m_temperature = 0.0;
    double m_humidity = 0.0;
    double m_co2 = 0.0;
    double m_pm25 = 0.0;
    bool   m_methane = false;
    double m_light = 0.0;
    bool   m_smoke = false;
    bool   m_fire = false;

    // 服务器连接信息
    QString m_serverIp;
    int     m_serverPort = 10086;
    int     m_roomId = 1;

    // 阈值配置（默认值）
    int    m_co2Max = 1000;
    int    m_pm25Max = 50;
    double m_tempMax = 30.0;
    double m_tempMin = 18.0;
    bool   m_methaneEnable = true;
    int    m_cooldownSeconds = 30;
    int    m_fanCooldownSeconds = 30;

    /** 更新传感器数据并发射通知 */
    void updateSensorValue(int sensorType, double value);
};

#endif // SENSORCONTROLLER_H
