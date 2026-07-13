#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpclient.h"
#include <QMessageBox>
#include <QJsonObject>
#include <QStatusBar>
#include <QDebug>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QDateTime>
#include <QSqlDriver>
#include <QSqlRecord>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tcpClient(new TcpClient(this))
    , serverIP("127.0.0.1")
    , serverPort(10086)  // 与服务器默认端口一致
    , currentSensorIndex(0)
    , autoRequestTimer(nullptr)
    , dbConnected(false)
    , historyWindow(nullptr)
    , chartView(nullptr)
    , series(nullptr)
    , axisX(nullptr)
    , axisY(nullptr)
    , allDataWindow(nullptr)
{
    ui->setupUi(this);

    // 初始化传感器类型列表（按顺序请求）
    sensorTypes = {
        20002,  // 温度
        20008,  // 湿度
        20001,  // CO2
        20004,  // PM2.5
        20005,  // 甲烷
        20006,  // 光照
        20007,  // 烟雾
        20003   // 火光
    };

    // 初始化传感器信息列表
    sensorInfoList = {
        {SENSOR_TEMPERATURE, "温度", "Temperature", "°C", "%.1f"},
        {SENSOR_HUMIDITY, "湿度", "Humidity", "%", "%.1f"},
        {SENSOR_CO2, "CO₂", "Co2", "ppm", "%.0f"},
        {SENSOR_PM25, "PM2.5", "PM25", "µg/m³", "%.1f"},
        {SENSOR_METHANE, "甲烷", "Methane", "%", "%.1f"},
        {SENSOR_LIGHT, "光照", "Light", "lux", "%.0f"},
        {SENSOR_SMOKE, "烟雾", "Smoke", "%", "%.1f"},
        {SENSOR_FIRE, "火光", "Fire", "", "%.0f"}
    };

    // 初始化传感器显示标签
    initSensorLabels();

    // 连接信号槽
    connect(tcpClient, &TcpClient::connected, this, &MainWindow::onConnected);
    connect(tcpClient, &TcpClient::disconnected, this, &MainWindow::onDisconnected);
    connect(tcpClient, &TcpClient::errorOccurred, this, &MainWindow::onError);
    connect(tcpClient, &TcpClient::dataReceived, this, &MainWindow::onDataReceived);

    // 设备控制
    connect(ui->btnLED2Set, &QPushButton::clicked, this, &MainWindow::onLED2SetClicked);
    connect(ui->btnRGBSet, &QPushButton::clicked, this, &MainWindow::onRGBSetClicked);
    connect(ui->btnFanOn, &QPushButton::clicked, this, &MainWindow::onFanOnClicked);
    connect(ui->btnFanOff, &QPushButton::clicked, this, &MainWindow::onFanOffClicked);
    connect(ui->btnAlarmOn, &QPushButton::clicked, this, &MainWindow::onAlarmOnClicked);
    connect(ui->btnAlarmOff, &QPushButton::clicked, this, &MainWindow::onAlarmOffClicked);
    connect(ui->btnAlarmLightOn, &QPushButton::clicked, this, &MainWindow::onAlarmLightOnClicked);
    connect(ui->btnAlarmLightOff, &QPushButton::clicked, this, &MainWindow::onAlarmLightOffClicked);

    // 传感器触发
    connect(ui->btnTemp, &QPushButton::clicked, this, &MainWindow::onTemperatureClicked);
    connect(ui->btnHumidity, &QPushButton::clicked, this, &MainWindow::onHumidityClicked);
    connect(ui->btnCo2, &QPushButton::clicked, this, &MainWindow::onCo2Clicked);
    connect(ui->btnPM25, &QPushButton::clicked, this, &MainWindow::onPM25Clicked);
    connect(ui->btnMethane, &QPushButton::clicked, this, &MainWindow::onMethaneClicked);
    connect(ui->btnLight, &QPushButton::clicked, this, &MainWindow::onLightClicked);
    connect(ui->btnSmoke, &QPushButton::clicked, this, &MainWindow::onSmokeClicked);
    connect(ui->btnFire, &QPushButton::clicked, this, &MainWindow::onFireClicked);

    // ========== 阈值设置信号连接 ==========
    connect(ui->btnApplyThreshold, &QPushButton::clicked,
            this, &MainWindow::onApplyThresholdClicked);
    connect(ui->btnLoadDefault, &QPushButton::clicked,
            this, &MainWindow::onLoadDefaultClicked);

    // ========== 新增：连接服务器指令信号 ==========
    connect(tcpClient, &TcpClient::serverCommandReceived,
            this, &MainWindow::onServerCommandReceived);

    // 动态创建风扇关闭延迟控件（添加到阈值设置区域）
    {
        // 在 groupBoxThreshold 的布局中查找 spinCooldown 所在行，在其后插入新行
        QLayout *layout = ui->groupBoxThreshold->layout();
        if (layout) {
            QHBoxLayout *row = new QHBoxLayout();
            QLabel *label = new QLabel("风扇关闭延迟(秒):");
            label->setStyleSheet("font-size: 13px;");
            spinFanCooldown = new QSpinBox();
            spinFanCooldown->setRange(0, 300);
            spinFanCooldown->setValue(30);
            spinFanCooldown->setSuffix(" 秒");
            spinFanCooldown->setToolTip("所有传感器数值恢复正常后，风扇继续运行N秒再关闭。设为0立即关闭。");
            row->addWidget(label);
            row->addWidget(spinFanCooldown);
            row->addStretch();
            layout->addItem(row);
        }
    }

    // 读取UI中的默认值到配置
    loadDefaultThresholds();

    // 查看历史数据按钮
    connect(ui->btnShowHistory, &QPushButton::clicked, this, &MainWindow::onShowHistoryClicked);

    // 初始禁用设备控制
    ui->groupBoxDevice->setEnabled(false);

    // 设置窗口标题
    setWindowTitle("🏠 智能家居网关");

    // 创建定时器用于自动请求
    autoRequestTimer = new QTimer(this);
    connect(autoRequestTimer, &QTimer::timeout, this, &MainWindow::requestNextSensor);

    // 初始化数据库连接
    initDatabase();

    // 创建历史数据显示窗口
    createHistoryWindow();
}

MainWindow::~MainWindow()
{
    stopAutoRequest();
    if (db.isOpen()) {
        db.close();
    }
    delete ui;
}

void MainWindow::initSensorLabels()
{
    // 使用UI中已经定义的标签
}

void MainWindow::setServerInfo(const QString &ip, quint16 port)
{
    serverIP = ip;
    serverPort = port;
}

void MainWindow::connectToServer()
{
    ui->labelStatus->setText("连接中...");
    ui->labelStatus->setStyleSheet("color: #ffa500;");

    tcpClient->connectToServer(serverIP, serverPort);

    QTimer::singleShot(5000, this, [this]() {
        if (!tcpClient->isConnected()) {
            ui->labelStatus->setText("❌ 连接超时");
            ui->labelStatus->setStyleSheet("color: #ff4444;");
            QMessageBox::critical(this, "错误", "连接服务器超时，请检查网络设置");
        }
    });
}

void MainWindow::updateConnectionStatus(bool connected)
{
    if (connected) {
        ui->labelStatus->setText("✅ 已连接");
        ui->labelStatus->setStyleSheet("color: #4ecdc4;");
        ui->groupBoxDevice->setEnabled(true);
        ui->groupBoxThreshold->setEnabled(true);
        statusBar()->showMessage("已连接到服务器", 3000);
        startAutoRequest();
    } else {
        ui->labelStatus->setText("❌ 未连接");
        ui->labelStatus->setStyleSheet("color: #ff4444;");
        ui->groupBoxDevice->setEnabled(false);
        ui->groupBoxThreshold->setEnabled(false);
        statusBar()->showMessage("已断开连接", 3000);
        stopAutoRequest();
    }
}

void MainWindow::startAutoRequest()
{
    if (!tcpClient->isConnected()) {
        qDebug() << "Cannot start auto request: not connected";
        return;
    }

    currentSensorIndex = 0;

    if (autoRequestTimer->isActive()) {
        autoRequestTimer->stop();
    }

    requestNextSensor();
    autoRequestTimer->start(60000);

    qDebug() << "🔄 Auto request started (every 60 seconds)";
}

void MainWindow::stopAutoRequest()
{
    if (autoRequestTimer) {
        autoRequestTimer->stop();
    }
    qDebug() << "🔄 Auto request stopped";
}

void MainWindow::requestNextSensor()
{
    if (!tcpClient->isConnected()) {
        qDebug() << "Cannot request sensor: not connected";
        return;
    }

    int type = sensorTypes[currentSensorIndex];

    QJsonObject cmd;
    cmd["type"] = type;

    tcpClient->sendCommand(cmd);

    QString sensorName;
    switch(type) {
    case 20001: sensorName = "CO2"; break;
    case 20002: sensorName = "温度"; break;
    case 20003: sensorName = "火光"; break;
    case 20004: sensorName = "PM2.5"; break;
    case 20005: sensorName = "甲烷"; break;
    case 20006: sensorName = "光照"; break;
    case 20007: sensorName = "烟雾"; break;
    case 20008: sensorName = "湿度"; break;
    default: sensorName = QString("未知(%1)").arg(type);
    }

    qDebug() << "📤 发送请求 [" << (currentSensorIndex + 1) << "/" << sensorTypes.size() << "]: " << sensorName;
    qDebug() << "📤 请求内容:" << cmd;

    statusBar()->showMessage(QString("请求: %1").arg(sensorName), 2000);

    currentSensorIndex++;
    if (currentSensorIndex >= sensorTypes.size()) {
        currentSensorIndex = 0;
    }
}

void MainWindow::sendCommand(const QJsonObject &cmd)
{
    if (!tcpClient->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器");
        return;
    }
    tcpClient->sendCommand(cmd);
    qDebug() << "📤 发送命令:" << cmd;
}

void MainWindow::updateSensorDisplay()
{
    qDebug() << "📊 更新传感器显示:";
    qDebug() << "  温度:" << sensorData.temperature;
    qDebug() << "  湿度:" << sensorData.humidity;
    qDebug() << "  CO2:" << sensorData.co2;
    qDebug() << "  PM2.5:" << sensorData.pm25;
    qDebug() << "  甲烷:" << sensorData.methane;
    qDebug() << "  光照:" << sensorData.light;
    qDebug() << "  烟雾:" << sensorData.smoke;
    qDebug() << "  火光:" << sensorData.fire;

    if (ui->labelTempValue) {
        ui->labelTempValue->setText(QString::number(sensorData.temperature, 'f', 1) + "°C");
    }

    if (ui->labelHumidityValue) {
        ui->labelHumidityValue->setText(QString::number(sensorData.humidity, 'f', 1) + "%");
    }

    if (ui->labelCo2Value) {
        ui->labelCo2Value->setText(QString::number(sensorData.co2, 'f', 0) + " ppm");
    }

    if (ui->labelPM25Value) {
        ui->labelPM25Value->setText(QString::number(sensorData.pm25, 'f', 1) + " µg/m³");
    }

    if (ui->labelMethaneValue) {
        ui->labelMethaneValue->setText(sensorData.methane > 0.5 ? "🔥 异常" : "✅ 正常");
    }

    if (ui->labelLightValue) {
        ui->labelLightValue->setText(QString::number(sensorData.light, 'f', 0) + " lux");
    }

    if (ui->labelSmokeValue) {
        ui->labelSmokeValue->setText(sensorData.smoke > 0.5 ? "🔥 异常" : "✅ 正常");
    }

    if (ui->labelFireValue) {
        ui->labelFireValue->setText(sensorData.fire > 0.5 ? "🔥 检测到" : "✅ 正常");
    }

    qDebug() << "📊 传感器显示更新完成";
}

// ==================== 数据库操作 ====================

bool MainWindow::initDatabase()
{
    // 检查MySQL驱动是否可用
    if (!QSqlDatabase::drivers().contains("QMYSQL")) {
        qDebug() << "❌ MySQL驱动不可用！";
        qDebug() << "可用驱动:" << QSqlDatabase::drivers();
        dbConnected = false;
        statusBar()->showMessage("⚠️ MySQL驱动不可用", 5000);
        return false;
    }

    db = QSqlDatabase::addDatabase("QMYSQL");

    // ========== 针对WSL的配置 ==========
    // 如果程序运行在WSL内部，使用 localhost
    // db.setHostName("localhost");
    // 如果从Windows连接WSL，使用WSL的IP地址
    db.setHostName("127.0.0.1");

    db.setDatabaseName("smarthome");
    db.setUserName("root");           // 修改为你的MySQL用户名
    db.setPassword("123456");         // 修改为你的MySQL密码
    db.setPort(13307);

    if (!db.open()) {
        qDebug() << "❌ 数据库连接失败:" << db.lastError().text();
        dbConnected = false;
        statusBar()->showMessage("⚠️ 数据库连接失败: " + db.lastError().text(), 5000);
        return false;
    }

    dbConnected = true;
    qDebug() << "✅ 数据库连接成功";
    statusBar()->showMessage("✅ 数据库连接成功", 3000);
    return true;
}

void MainWindow::querySensorHistory(SensorType type)
{
    if (!dbConnected || !db.isOpen()) {
        qDebug() << "❌ 数据库未连接，尝试重新连接...";
        if (!initDatabase()) {
            QMessageBox::warning(this, "错误", "无法连接数据库，请检查数据库配置");
            return;
        }
    }

    // 根据传感器类型获取表名
    QString tableName;
    QString sensorName;
    QString unit;
    for (const auto &info : sensorInfoList) {
        if (info.type == type) {
            tableName = info.tableName;
            sensorName = info.name;
            unit = info.unit;
            break;
        }
    }

    if (tableName.isEmpty()) {
        qDebug() << "❌ 未知传感器类型:" << type;
        return;
    }

    // 检查表是否存在
    QStringList tables = db.tables();
    if (!tables.contains(tableName, Qt::CaseInsensitive)) {
        qDebug() << "❌ 表不存在:" << tableName;
        QMessageBox::warning(this, "错误", QString("数据表 '%1' 不存在").arg(tableName));
        return;
    }

    // 查询最近50条数据
    QString queryStr = QString(
                           "SELECT * FROM `%1` ORDER BY id DESC LIMIT 50"
                           ).arg(tableName);

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qDebug() << "❌ 查询失败:" << query.lastError().text();
        QMessageBox::warning(this, "错误", "查询数据失败: " + query.lastError().text());
        return;
    }

    // 清空系列数据
    if (series) {
        series->clear();
    }

    // 收集数据
    QList<QPair<QDateTime, double>> dataPoints;

    // 获取字段名
    QString valueField = "value";
    QString timeField = "timestamp";

    // 检查字段是否存在
    QSqlRecord record = query.record();
    if (record.indexOf("value") == -1) {
        if (record.indexOf("data") != -1) valueField = "data";
        else if (record.indexOf("val") != -1) valueField = "val";
    }
    if (record.indexOf("timestamp") == -1) {
        if (record.indexOf("time") != -1) timeField = "time";
        else if (record.indexOf("datetime") != -1) timeField = "datetime";
        else if (record.indexOf("create_time") != -1) timeField = "create_time";
    }

    qDebug() << "📊 使用字段: value=" << valueField << ", time=" << timeField;

    // 重新查询
    queryStr = QString(
                   "SELECT * FROM `%1` ORDER BY id DESC LIMIT 50"
                   ).arg(tableName);

    if (!query.exec(queryStr)) {
        qDebug() << "❌ 重新查询失败:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        QDateTime timestamp;
        // 尝试获取时间戳
        QVariant timeVar = query.value(timeField);
        if (timeVar.canConvert<QDateTime>()) {
            timestamp = timeVar.toDateTime();
        } else if (timeVar.typeId() == QMetaType::QString) {
            timestamp = QDateTime::fromString(timeVar.toString(), "yyyy-MM-dd hh:mm:ss");
            if (!timestamp.isValid()) {
                timestamp = QDateTime::fromString(timeVar.toString(), "yyyy-MM-dd HH:mm:ss");
            }
        } else if (timeVar.typeId() == QMetaType::Int || timeVar.typeId() == QMetaType::LongLong) {
            qint64 ts = timeVar.toLongLong();
            if (ts > 10000000000) {
                timestamp = QDateTime::fromMSecsSinceEpoch(ts);
            } else {
                timestamp = QDateTime::fromSecsSinceEpoch(ts);
            }
        }

        if (!timestamp.isValid()) {
            timestamp = QDateTime::currentDateTime().addSecs(-dataPoints.size() * 10);
        }

        double value = query.value(valueField).toDouble();
        dataPoints.prepend({timestamp, value});
    }

    // 如果数据点数少于2，生成示例数据
    if (dataPoints.size() < 2) {
        qDebug() << "⚠️ 数据点不足，使用示例数据";
        dataPoints.clear();
        QRandomGenerator *gen = QRandomGenerator::global();
        for (int i = 0; i < 30; i++) {
            double val;
            switch(type) {
            case SENSOR_TEMPERATURE:
                val = 20.0 + (gen->generateDouble() * 3.0);
                break;
            case SENSOR_HUMIDITY:
                val = 40.0 + (gen->generateDouble() * 4.0);
                break;
            case SENSOR_CO2:
                val = 400 + gen->bounded(200);
                break;
            case SENSOR_PM25:
                val = 10 + gen->bounded(100);
                break;
            case SENSOR_METHANE:
                val = gen->generateDouble();
                break;
            case SENSOR_LIGHT:
                val = 100 + gen->bounded(500);
                break;
            case SENSOR_SMOKE:
                val = gen->generateDouble();
                break;
            case SENSOR_FIRE:
                val = gen->bounded(2);
                break;
            default:
                val = gen->bounded(100);
            }
            QDateTime dt = QDateTime::currentDateTime().addSecs(-(30 - i) * 10);
            dataPoints.append({dt, val});
        }
    }

    // 将数据添加到系列
    if (series) {
        for (const auto &point : dataPoints) {
            qreal x = point.first.toMSecsSinceEpoch();
            qreal y = point.second;
            series->append(x, y);
            qDebug() << "  数据点:" << point.first.toString("hh:mm:ss") << "=" << y;
        }
    }

    // 更新图表标题和轴
    QString title = QString("%1 历史数据").arg(sensorName);
    updateChart(title, unit);

    qDebug() << "📊 加载了" << dataPoints.size() << "个数据点";
}

void MainWindow::updateChart(const QString &title, const QString &unit)
{
    if (!chartView || !series) {
        qDebug() << "❌ 图表组件未初始化";
        return;
    }

    QChart *chart = chartView->chart();
    if (!chart) {
        qDebug() << "❌ Chart对象为空";
        return;
    }

    // 更新标题
    chart->setTitle(title);

    // 更新Y轴标签
    if (axisY) {
        QString labelFormat = unit.isEmpty() ? "%.1f" : "%.1f " + unit;
        axisY->setLabelFormat(labelFormat);
        axisY->setTitleText("数值 (" + unit + ")");
    }

    // 更新X轴
    if (axisX) {
        axisX->setFormat("hh:mm:ss");
        axisX->setTitleText("时间");
    }

    // 调整Y轴范围
    if (series && series->count() > 0) {
        double minY = series->at(0).y();
        double maxY = series->at(0).y();
        for (int i = 0; i < series->count(); i++) {
            double y = series->at(i).y();
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;
        }
        double padding = (maxY - minY) * 0.1;
        if (padding < 1) padding = 1;
        axisY->setRange(minY - padding, maxY + padding);
    }

    // 调整X轴范围
    if (axisX && series && series->count() > 0) {
        qreal minX = series->at(0).x();
        qreal maxX = series->at(0).x();
        for (int i = 0; i < series->count(); i++) {
            qreal x = series->at(i).x();
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
        }
        axisX->setRange(QDateTime::fromMSecsSinceEpoch(minX), QDateTime::fromMSecsSinceEpoch(maxX));
    }

    chartView->update();
}

// ==================== 一周历史数据查询 ====================

void MainWindow::queryWeekHistory(SensorType type)
{
    if (!dbConnected || !db.isOpen()) {
        qDebug() << "❌ 数据库未连接，尝试重新连接...";
        if (!initDatabase()) {
            QMessageBox::warning(this, "错误", "无法连接数据库，请检查数据库配置");
            return;
        }
    }

    // 根据传感器类型获取表名
    QString tableName;
    QString sensorName;
    QString unit;
    for (const auto &info : sensorInfoList) {
        if (info.type == type) {
            tableName = info.tableName;
            sensorName = info.name;
            unit = info.unit;
            break;
        }
    }

    if (tableName.isEmpty()) {
        qDebug() << "❌ 未知传感器类型:" << type;
        return;
    }

    // 检查表是否存在
    QStringList tables = db.tables();
    if (!tables.contains(tableName, Qt::CaseInsensitive)) {
        qDebug() << "❌ 表不存在:" << tableName;
        QMessageBox::warning(this, "错误", QString("数据表 '%1' 不存在").arg(tableName));
        return;
    }

    // 探测字段名
    QSqlQuery testQuery;
    QString testStr = QString("SELECT * FROM `%1` LIMIT 1").arg(tableName);
    if (!testQuery.exec(testStr)) {
        qDebug() << "❌ 探测查询失败:" << testQuery.lastError().text();
        return;
    }

    QString valueField = "value";
    QString timeField = "timestamp";
    QSqlRecord record = testQuery.record();
    if (record.indexOf("value") == -1) {
        if (record.indexOf("data") != -1) valueField = "data";
        else if (record.indexOf("val") != -1) valueField = "val";
    }
    if (record.indexOf("timestamp") == -1) {
        if (record.indexOf("time") != -1) timeField = "time";
        else if (record.indexOf("datetime") != -1) timeField = "datetime";
        else if (record.indexOf("create_time") != -1) timeField = "create_time";
    }

    // 计算7天前的时间阈值
    QDateTime sevenDaysAgo = QDateTime::currentDateTime().addDays(-7);
    QString thresholdStr = sevenDaysAgo.toString("yyyy-MM-dd hh:mm:ss");

    // 查询一周内的全部数据
    QString queryStr = QString(
        "SELECT * FROM `%1` WHERE `%2` >= '%3' ORDER BY `%2` ASC"
    ).arg(tableName).arg(timeField).arg(thresholdStr);

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qDebug() << "❌ 一周数据查询失败:" << query.lastError().text();
        QMessageBox::warning(this, "错误", "查询一周数据失败: " + query.lastError().text());
        return;
    }

    // 清空系列数据
    if (series) {
        series->clear();
    }

    // 收集数据
    QList<QPair<QDateTime, double>> dataPoints;

    while (query.next()) {
        QDateTime timestamp;
        QVariant timeVar = query.value(timeField);
        if (timeVar.canConvert<QDateTime>()) {
            timestamp = timeVar.toDateTime();
        } else if (timeVar.typeId() == QMetaType::QString) {
            timestamp = QDateTime::fromString(timeVar.toString(), "yyyy-MM-dd hh:mm:ss");
            if (!timestamp.isValid()) {
                timestamp = QDateTime::fromString(timeVar.toString(), "yyyy-MM-dd HH:mm:ss");
            }
        } else if (timeVar.typeId() == QMetaType::Int || timeVar.typeId() == QMetaType::LongLong) {
            qint64 ts = timeVar.toLongLong();
            if (ts > 10000000000) {
                timestamp = QDateTime::fromMSecsSinceEpoch(ts);
            } else {
                timestamp = QDateTime::fromSecsSinceEpoch(ts);
            }
        }

        if (!timestamp.isValid()) {
            timestamp = QDateTime::currentDateTime().addSecs(-dataPoints.size() * 600);
        }

        double value = query.value(valueField).toDouble();
        dataPoints.append({timestamp, value});
    }

    // 数据不足时生成模拟一周数据
    if (dataPoints.size() < 2) {
        qDebug() << "⚠️ 一周数据点不足，使用示例数据";
        dataPoints.clear();
        QRandomGenerator *gen = QRandomGenerator::global();
        // 生成过去7天，每天约4个数据点（共28个）
        for (int day = 6; day >= 0; day--) {
            for (int h = 0; h < 24; h += 6) {
                double val;
                switch(type) {
                case SENSOR_TEMPERATURE:
                    val = 20.0 + (gen->generateDouble() * 5.0);
                    break;
                case SENSOR_HUMIDITY:
                    val = 40.0 + (gen->generateDouble() * 10.0);
                    break;
                case SENSOR_CO2:
                    val = 400 + gen->bounded(300);
                    break;
                case SENSOR_PM25:
                    val = 10 + gen->bounded(60);
                    break;
                case SENSOR_METHANE:
                    val = gen->generateDouble() * 0.3;
                    break;
                case SENSOR_LIGHT:
                    val = 100 + gen->bounded(500);
                    break;
                case SENSOR_SMOKE:
                    val = gen->generateDouble() * 0.3;
                    break;
                case SENSOR_FIRE:
                    val = gen->bounded(2);
                    break;
                default:
                    val = gen->bounded(100);
                }
                QDateTime dt = QDateTime::currentDateTime().addDays(-day).addSecs(-h * 3600);
                dataPoints.append({dt, val});
            }
        }
    }

    // 将数据添加到系列
    if (series) {
        for (const auto &point : dataPoints) {
            qreal x = point.first.toMSecsSinceEpoch();
            qreal y = point.second;
            series->append(x, y);
            qDebug() << "  一周数据点:" << point.first.toString("MM-dd hh:mm") << "=" << y;
        }
    }

    // X轴格式改为"MM-dd hh:mm"（跨天显示）
    if (axisX) {
        axisX->setFormat("MM-dd hh:mm");
    }

    // 更新图表
    QString title = QString("%1 一周历史数据").arg(sensorName);
    updateChart(title, unit);

    qDebug() << "📊 一周数据加载完成，共" << dataPoints.size() << "个数据点";
}

// ==================== 月度日均值查询 ====================

void MainWindow::queryMonthAvgHistory(SensorType type)
{
    if (!dbConnected || !db.isOpen()) {
        qDebug() << "❌ 数据库未连接，尝试重新连接...";
        if (!initDatabase()) {
            QMessageBox::warning(this, "错误", "无法连接数据库，请检查数据库配置");
            return;
        }
    }

    // 根据传感器类型获取表名
    QString tableName;
    QString sensorName;
    QString unit;
    for (const auto &info : sensorInfoList) {
        if (info.type == type) {
            tableName = info.tableName;
            sensorName = info.name;
            unit = info.unit;
            break;
        }
    }

    if (tableName.isEmpty()) {
        qDebug() << "❌ 未知传感器类型:" << type;
        return;
    }

    // 检查表是否存在
    QStringList tables = db.tables();
    if (!tables.contains(tableName, Qt::CaseInsensitive)) {
        qDebug() << "❌ 表不存在:" << tableName;
        QMessageBox::warning(this, "错误", QString("数据表 '%1' 不存在").arg(tableName));
        return;
    }

    // 探测字段名
    QSqlQuery testQuery;
    QString testStr = QString("SELECT * FROM `%1` LIMIT 1").arg(tableName);
    if (!testQuery.exec(testStr)) {
        qDebug() << "❌ 探测查询失败:" << testQuery.lastError().text();
        return;
    }

    QString valueField = "value";
    QString timeField = "timestamp";
    QSqlRecord record = testQuery.record();
    if (record.indexOf("value") == -1) {
        if (record.indexOf("data") != -1) valueField = "data";
        else if (record.indexOf("val") != -1) valueField = "val";
    }
    if (record.indexOf("timestamp") == -1) {
        if (record.indexOf("time") != -1) timeField = "time";
        else if (record.indexOf("datetime") != -1) timeField = "datetime";
        else if (record.indexOf("create_time") != -1) timeField = "create_time";
    }

    // 计算30天前的时间阈值
    QDateTime thirtyDaysAgo = QDateTime::currentDateTime().addDays(-30);
    QString thresholdStr = thirtyDaysAgo.toString("yyyy-MM-dd hh:mm:ss");

    // 查询月度日均值：按天分组取平均值
    QString queryStr = QString(
        "SELECT DATE(`%1`) as query_day, AVG(`%2`) as avg_val "
        "FROM `%3` WHERE `%1` >= '%4' "
        "GROUP BY DATE(`%1`) ORDER BY query_day ASC"
    ).arg(timeField).arg(valueField).arg(tableName).arg(thresholdStr);

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qDebug() << "❌ 月度均值查询失败:" << query.lastError().text();
        QMessageBox::warning(this, "错误", "查询月度均值失败: " + query.lastError().text());
        return;
    }

    // 清空系列数据
    if (series) {
        series->clear();
    }

    // 收集数据
    QList<QPair<QDateTime, double>> dataPoints;

    while (query.next()) {
        // 读取日期字段
        QVariant dayVar = query.value("query_day");
        QDateTime dayTimestamp;
        if (dayVar.canConvert<QDateTime>()) {
            dayTimestamp = dayVar.toDateTime();
        } else if (dayVar.typeId() == QMetaType::QString) {
            QString dayStr = dayVar.toString();
            dayTimestamp = QDateTime::fromString(dayStr, "yyyy-MM-dd");
            if (!dayTimestamp.isValid()) {
                dayTimestamp = QDateTime::fromString(dayStr, "yyyy-M-d");
            }
            // 设置为当天中午12:00，使图表点居中对齐
            if (dayTimestamp.isValid()) {
                dayTimestamp.setTime(QTime(12, 0, 0));
            }
        }

        if (!dayTimestamp.isValid()) {
            dayTimestamp = QDateTime::currentDateTime().addDays(-dataPoints.size());
            dayTimestamp.setTime(QTime(12, 0, 0));
        }

        double avgValue = query.value("avg_val").toDouble();
        dataPoints.append({dayTimestamp, avgValue});
    }

    // 数据不足时生成模拟月度均值数据
    if (dataPoints.size() < 2) {
        qDebug() << "⚠️ 月度均值数据点不足，使用示例数据";
        dataPoints.clear();
        QRandomGenerator *gen = QRandomGenerator::global();
        // 生成过去30天，每天一个均值点
        for (int day = 29; day >= 0; day--) {
            double val;
            switch(type) {
            case SENSOR_TEMPERATURE:
                val = 22.0 + (gen->generateDouble() * 6.0);
                break;
            case SENSOR_HUMIDITY:
                val = 42.0 + (gen->generateDouble() * 12.0);
                break;
            case SENSOR_CO2:
                val = 450 + gen->bounded(250);
                break;
            case SENSOR_PM25:
                val = 15 + gen->bounded(45);
                break;
            case SENSOR_METHANE:
                val = gen->generateDouble() * 0.2;
                break;
            case SENSOR_LIGHT:
                val = 150 + gen->bounded(400);
                break;
            case SENSOR_SMOKE:
                val = gen->generateDouble() * 0.2;
                break;
            case SENSOR_FIRE:
                val = gen->bounded(2);
                break;
            default:
                val = gen->bounded(100);
            }
            QDateTime dt = QDateTime::currentDateTime().addDays(-day);
            dt.setTime(QTime(12, 0, 0));  // 每天中午12点
            dataPoints.append({dt, val});
        }
    }

    // 将数据添加到系列
    if (series) {
        for (const auto &point : dataPoints) {
            qreal x = point.first.toMSecsSinceEpoch();
            qreal y = point.second;
            series->append(x, y);
            qDebug() << "  月度均值:" << point.first.toString("MM-dd") << "=" << y;
        }
    }

    // X轴格式改为"MM-dd"（每天一个点）
    if (axisX) {
        axisX->setFormat("MM-dd");
    }

    // 更新图表
    QString title = QString("%1 月度日均值").arg(sensorName);
    updateChart(title, unit);

    qDebug() << "📊 月度日均值加载完成，共" << dataPoints.size() << "个数据点";
}

// ==================== 一周日均值查询 ====================

void MainWindow::queryWeekAvgHistory(SensorType type)
{
    if (!dbConnected || !db.isOpen()) {
        qDebug() << "❌ 数据库未连接，尝试重新连接...";
        if (!initDatabase()) {
            QMessageBox::warning(this, "错误", "无法连接数据库，请检查数据库配置");
            return;
        }
    }

    // 根据传感器类型获取表名
    QString tableName;
    QString sensorName;
    QString unit;
    for (const auto &info : sensorInfoList) {
        if (info.type == type) {
            tableName = info.tableName;
            sensorName = info.name;
            unit = info.unit;
            break;
        }
    }

    if (tableName.isEmpty()) {
        qDebug() << "❌ 未知传感器类型:" << type;
        return;
    }

    QStringList tables = db.tables();
    if (!tables.contains(tableName, Qt::CaseInsensitive)) {
        qDebug() << "❌ 表不存在:" << tableName;
        QMessageBox::warning(this, "错误", QString("数据表 '%1' 不存在").arg(tableName));
        return;
    }

    // 探测字段名
    QSqlQuery testQuery;
    QString testStr = QString("SELECT * FROM `%1` LIMIT 1").arg(tableName);
    if (!testQuery.exec(testStr)) {
        qDebug() << "❌ 探测查询失败:" << testQuery.lastError().text();
        return;
    }

    QString valueField = "value";
    QString timeField = "timestamp";
    QSqlRecord record = testQuery.record();
    if (record.indexOf("value") == -1) {
        if (record.indexOf("data") != -1) valueField = "data";
        else if (record.indexOf("val") != -1) valueField = "val";
    }
    if (record.indexOf("timestamp") == -1) {
        if (record.indexOf("time") != -1) timeField = "time";
        else if (record.indexOf("datetime") != -1) timeField = "datetime";
        else if (record.indexOf("create_time") != -1) timeField = "create_time";
    }

    // 计算7天前的时间阈值
    QDateTime sevenDaysAgo = QDateTime::currentDateTime().addDays(-7);
    QString thresholdStr = sevenDaysAgo.toString("yyyy-MM-dd hh:mm:ss");

    // 查询一周内每日均值
    QString queryStr = QString(
        "SELECT DATE(`%1`) as query_day, AVG(`%2`) as avg_val "
        "FROM `%3` WHERE `%1` >= '%4' "
        "GROUP BY DATE(`%1`) ORDER BY query_day ASC"
    ).arg(timeField).arg(valueField).arg(tableName).arg(thresholdStr);

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qDebug() << "❌ 一周均值查询失败:" << query.lastError().text();
        QMessageBox::warning(this, "错误", "查询一周均值失败: " + query.lastError().text());
        return;
    }

    if (series) {
        series->clear();
    }

    QList<QPair<QDateTime, double>> dataPoints;

    while (query.next()) {
        QVariant dayVar = query.value("query_day");
        QDateTime dayTimestamp;
        if (dayVar.canConvert<QDateTime>()) {
            dayTimestamp = dayVar.toDateTime();
        } else if (dayVar.typeId() == QMetaType::QString) {
            QString dayStr = dayVar.toString();
            dayTimestamp = QDateTime::fromString(dayStr, "yyyy-MM-dd");
            if (!dayTimestamp.isValid()) {
                dayTimestamp = QDateTime::fromString(dayStr, "yyyy-M-d");
            }
            if (dayTimestamp.isValid()) {
                dayTimestamp.setTime(QTime(12, 0, 0));
            }
        }

        if (!dayTimestamp.isValid()) {
            dayTimestamp = QDateTime::currentDateTime().addDays(-dataPoints.size());
            dayTimestamp.setTime(QTime(12, 0, 0));
        }

        double avgValue = query.value("avg_val").toDouble();
        dataPoints.append({dayTimestamp, avgValue});
    }

    // 数据不足时生成模拟一周每日均值
    if (dataPoints.size() < 2) {
        qDebug() << "⚠️ 一周均值数据点不足，使用示例数据";
        dataPoints.clear();
        QRandomGenerator *gen = QRandomGenerator::global();
        for (int day = 6; day >= 0; day--) {
            double val;
            switch(type) {
            case SENSOR_TEMPERATURE:
                val = 20.0 + (gen->generateDouble() * 5.0);
                break;
            case SENSOR_HUMIDITY:
                val = 40.0 + (gen->generateDouble() * 10.0);
                break;
            case SENSOR_CO2:
                val = 400 + gen->bounded(300);
                break;
            case SENSOR_PM25:
                val = 10 + gen->bounded(60);
                break;
            case SENSOR_METHANE:
                val = gen->generateDouble() * 0.3;
                break;
            case SENSOR_LIGHT:
                val = 100 + gen->bounded(500);
                break;
            case SENSOR_SMOKE:
                val = gen->generateDouble() * 0.3;
                break;
            case SENSOR_FIRE:
                val = gen->bounded(2);
                break;
            default:
                val = gen->bounded(100);
            }
            QDateTime dt = QDateTime::currentDateTime().addDays(-day);
            dt.setTime(QTime(12, 0, 0));
            dataPoints.append({dt, val});
        }
    }

    if (series) {
        for (const auto &point : dataPoints) {
            series->append(point.first.toMSecsSinceEpoch(), point.second);
            qDebug() << "  一周均值:" << point.first.toString("MM-dd") << "=" << point.second;
        }
    }

    if (axisX) {
        axisX->setFormat("MM-dd");
    }

    QString title = QString("%1 一周日均值").arg(sensorName);
    updateChart(title, unit);

    qDebug() << "📊 一周日均值加载完成，共" << dataPoints.size() << "个数据点";
}

// ==================== 一月全部数据查询 ====================

void MainWindow::queryMonthHistory(SensorType type)
{
    if (!dbConnected || !db.isOpen()) {
        qDebug() << "❌ 数据库未连接，尝试重新连接...";
        if (!initDatabase()) {
            QMessageBox::warning(this, "错误", "无法连接数据库，请检查数据库配置");
            return;
        }
    }

    QString tableName;
    QString sensorName;
    QString unit;
    for (const auto &info : sensorInfoList) {
        if (info.type == type) {
            tableName = info.tableName;
            sensorName = info.name;
            unit = info.unit;
            break;
        }
    }

    if (tableName.isEmpty()) {
        qDebug() << "❌ 未知传感器类型:" << type;
        return;
    }

    QStringList tables = db.tables();
    if (!tables.contains(tableName, Qt::CaseInsensitive)) {
        qDebug() << "❌ 表不存在:" << tableName;
        QMessageBox::warning(this, "错误", QString("数据表 '%1' 不存在").arg(tableName));
        return;
    }

    QSqlQuery testQuery;
    QString testStr = QString("SELECT * FROM `%1` LIMIT 1").arg(tableName);
    if (!testQuery.exec(testStr)) {
        qDebug() << "❌ 探测查询失败:" << testQuery.lastError().text();
        return;
    }

    QString valueField = "value";
    QString timeField = "timestamp";
    QSqlRecord record = testQuery.record();
    if (record.indexOf("value") == -1) {
        if (record.indexOf("data") != -1) valueField = "data";
        else if (record.indexOf("val") != -1) valueField = "val";
    }
    if (record.indexOf("timestamp") == -1) {
        if (record.indexOf("time") != -1) timeField = "time";
        else if (record.indexOf("datetime") != -1) timeField = "datetime";
        else if (record.indexOf("create_time") != -1) timeField = "create_time";
    }

    // 计算30天前的时间阈值
    QDateTime thirtyDaysAgo = QDateTime::currentDateTime().addDays(-30);
    QString thresholdStr = thirtyDaysAgo.toString("yyyy-MM-dd hh:mm:ss");

    // 查询一月内全部数据
    QString queryStr = QString(
        "SELECT * FROM `%1` WHERE `%2` >= '%3' ORDER BY `%2` ASC"
    ).arg(tableName).arg(timeField).arg(thresholdStr);

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qDebug() << "❌ 一月数据查询失败:" << query.lastError().text();
        QMessageBox::warning(this, "错误", "查询一月数据失败: " + query.lastError().text());
        return;
    }

    if (series) {
        series->clear();
    }

    QList<QPair<QDateTime, double>> dataPoints;

    while (query.next()) {
        QDateTime timestamp;
        QVariant timeVar = query.value(timeField);
        if (timeVar.canConvert<QDateTime>()) {
            timestamp = timeVar.toDateTime();
        } else if (timeVar.typeId() == QMetaType::QString) {
            timestamp = QDateTime::fromString(timeVar.toString(), "yyyy-MM-dd hh:mm:ss");
            if (!timestamp.isValid()) {
                timestamp = QDateTime::fromString(timeVar.toString(), "yyyy-MM-dd HH:mm:ss");
            }
        } else if (timeVar.typeId() == QMetaType::Int || timeVar.typeId() == QMetaType::LongLong) {
            qint64 ts = timeVar.toLongLong();
            if (ts > 10000000000) {
                timestamp = QDateTime::fromMSecsSinceEpoch(ts);
            } else {
                timestamp = QDateTime::fromSecsSinceEpoch(ts);
            }
        }

        if (!timestamp.isValid()) {
            timestamp = QDateTime::currentDateTime().addSecs(-dataPoints.size() * 3600);
        }

        double value = query.value(valueField).toDouble();
        dataPoints.append({timestamp, value});
    }

    // 数据不足时生成模拟一月数据
    if (dataPoints.size() < 2) {
        qDebug() << "⚠️ 一月数据点不足，使用示例数据";
        dataPoints.clear();
        QRandomGenerator *gen = QRandomGenerator::global();
        // 生成过去30天，每天约2个数据点
        for (int day = 29; day >= 0; day--) {
            for (int h = 0; h < 24; h += 12) {
                double val;
                switch(type) {
                case SENSOR_TEMPERATURE:
                    val = 20.0 + (gen->generateDouble() * 6.0);
                    break;
                case SENSOR_HUMIDITY:
                    val = 40.0 + (gen->generateDouble() * 12.0);
                    break;
                case SENSOR_CO2:
                    val = 400 + gen->bounded(300);
                    break;
                case SENSOR_PM25:
                    val = 10 + gen->bounded(60);
                    break;
                case SENSOR_METHANE:
                    val = gen->generateDouble() * 0.3;
                    break;
                case SENSOR_LIGHT:
                    val = 100 + gen->bounded(500);
                    break;
                case SENSOR_SMOKE:
                    val = gen->generateDouble() * 0.3;
                    break;
                case SENSOR_FIRE:
                    val = gen->bounded(2);
                    break;
                default:
                    val = gen->bounded(100);
                }
                QDateTime dt = QDateTime::currentDateTime().addDays(-day).addSecs(-h * 3600);
                dataPoints.append({dt, val});
            }
        }
    }

    if (series) {
        for (const auto &point : dataPoints) {
            series->append(point.first.toMSecsSinceEpoch(), point.second);
            qDebug() << "  一月数据:" << point.first.toString("MM-dd hh:mm") << "=" << point.second;
        }
    }

    // X轴格式改为"MM-dd"（跨天显示但数据点多）
    if (axisX) {
        axisX->setFormat("MM-dd");
    }

    QString title = QString("%1 一月历史数据").arg(sensorName);
    updateChart(title, unit);

    qDebug() << "📊 一月数据加载完成，共" << dataPoints.size() << "个数据点";
}

// ==================== 历史数据窗口 ====================

void MainWindow::createHistoryWindow()
{
    // 创建历史数据窗口
    historyWindow = new QWidget();
    historyWindow->setWindowTitle("📊 传感器历史数据");
    historyWindow->setMinimumSize(900, 600);
    historyWindow->setStyleSheet(R"(
        QWidget {
            background-color: #f5f5f5;
        }
        QGroupBox {
            background-color: #ffffff;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            margin-top: 12px;
            padding: 8px;
            font-weight: bold;
        }
        QGroupBox::title {
            color: #333333;
            font-size: 14px;
            font-weight: bold;
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 8px;
        }
        QPushButton {
            border: none;
            border-radius: 6px;
            padding: 10px 16px;
            font-weight: bold;
            font-size: 13px;
            min-height: 20px;
        }
        QPushButton:hover {
            transform: scale(1.02);
        }
        QPushButton:pressed {
            transform: scale(0.98);
        }
        QPushButton#btnTempHistory {
            background-color: #e94560;
            color: white;
        }
        QPushButton#btnTempHistory:hover {
            background-color: #c73e54;
        }
        QPushButton#btnHumidityHistory {
            background-color: #4dabf7;
            color: white;
        }
        QPushButton#btnHumidityHistory:hover {
            background-color: #3a8fd4;
        }
        QPushButton#btnCo2History {
            background-color: #20c997;
            color: white;
        }
        QPushButton#btnCo2History:hover {
            background-color: #1aa87a;
        }
        QPushButton#btnPM25History {
            background-color: #ff922b;
            color: white;
        }
        QPushButton#btnPM25History:hover {
            background-color: #e07a1f;
        }
        QPushButton#btnMethaneHistory {
            background-color: #fcc419;
            color: #333;
        }
        QPushButton#btnMethaneHistory:hover {
            background-color: #e0ac12;
        }
        QPushButton#btnLightHistory {
            background-color: #f59f00;
            color: white;
        }
        QPushButton#btnLightHistory:hover {
            background-color: #d4880f;
        }
        QPushButton#btnSmokeHistory {
            background-color: #868e96;
            color: white;
        }
        QPushButton#btnSmokeHistory:hover {
            background-color: #6c757d;
        }
        QPushButton#btnFireHistory {
            background-color: #e03131;
            color: white;
        }
        QPushButton#btnFireHistory:hover {
            background-color: #c02525;
        }
        QPushButton#btnCloseHistory {
            background-color: #e9ecef;
            color: #333333;
        }
        QPushButton#btnCloseHistory:hover {
            background-color: #dee2e6;
        }
        QChartView {
            background-color: #ffffff;
            border-radius: 8px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(historyWindow);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // 传感器按钮区域
    QGroupBox *buttonGroup = new QGroupBox("选择传感器查看历史数据");
    QGridLayout *buttonLayout = new QGridLayout(buttonGroup);
    buttonLayout->setSpacing(10);

    // 创建8个传感器按钮
    QStringList sensorNames = {"温度", "湿度", "CO₂", "PM2.5", "甲烷", "光照", "烟雾", "火光"};
    QStringList objectNames = {"btnTempHistory", "btnHumidityHistory", "btnCo2History",
                               "btnPM25History", "btnMethaneHistory", "btnLightHistory",
                               "btnSmokeHistory", "btnFireHistory"};
    QStringList icons = {"🌡️", "💧", "🫧", "🌫️", "🔥", "☀️", "💨", "🔥"};

    for (int i = 0; i < 8; i++) {
        QPushButton *btn = new QPushButton(icons[i] + " " + sensorNames[i]);
        btn->setObjectName(objectNames[i]);
        btn->setMinimumHeight(45);
        btn->setProperty("sensorIndex", i);
        connect(btn, &QPushButton::clicked, this, &MainWindow::onHistoryButtonClicked);
        buttonLayout->addWidget(btn, i / 4, i % 4);
    }

    mainLayout->addWidget(buttonGroup);

    // 图表区域
    QGroupBox *chartGroup = new QGroupBox("数据图表");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartGroup);

    // 创建图表
    QChart *chart = new QChart();
    chart->setTitle("请点击上方按钮选择传感器查看历史数据");
    chart->setAnimationOptions(QChart::AllAnimations);
    chart->setTheme(QChart::ChartThemeLight);
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // 创建系列
    series = new QLineSeries();
    series->setName("传感器数据");
    series->setPointsVisible(true);
    series->setPointLabelsVisible(false);

    // 添加系列到图表
    chart->addSeries(series);

    // 创建坐标轴
    axisX = new QDateTimeAxis();
    axisX->setTitleText("时间");
    axisX->setFormat("hh:mm:ss");
    axisX->setGridLineVisible(true);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setTitleText("数值");
    axisY->setLabelFormat("%.1f");
    axisY->setGridLineVisible(true);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // 创建图表视图
    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(400);
    chartView->setRubberBand(QChartView::RectangleRubberBand);

    chartLayout->addWidget(chartView);
    mainLayout->addWidget(chartGroup);

    // 底部按钮区域
    QHBoxLayout *buttonLayoutBottom = new QHBoxLayout();
    buttonLayoutBottom->addStretch();

    // "一周数据"按钮
    QPushButton *weekBtn = new QPushButton("📅 一周数据");
    weekBtn->setObjectName("btnWeekData");
    weekBtn->setMinimumSize(120, 40);
    weekBtn->setStyleSheet(R"(
        QPushButton#btnWeekData {
            background-color: #4dabf7;
            color: white;
            border: none;
            border-radius: 6px;
            font-weight: bold;
        }
        QPushButton#btnWeekData:hover {
            background-color: #3a8fd4;
        }
    )");
    connect(weekBtn, &QPushButton::clicked, this, &MainWindow::onShowWeekDataClicked);
    buttonLayoutBottom->addWidget(weekBtn);

    buttonLayoutBottom->addSpacing(10);

    // "一周均值"按钮
    QPushButton *weekAvgBtn = new QPushButton("📅 一周均值");
    weekAvgBtn->setObjectName("btnWeekAvg");
    weekAvgBtn->setMinimumSize(120, 40);
    weekAvgBtn->setStyleSheet(R"(
        QPushButton#btnWeekAvg {
            background-color: #20c997;
            color: white;
            border: none;
            border-radius: 6px;
            font-weight: bold;
        }
        QPushButton#btnWeekAvg:hover {
            background-color: #1aa87a;
        }
    )");
    connect(weekAvgBtn, &QPushButton::clicked, this, &MainWindow::onShowWeekAvgClicked);
    buttonLayoutBottom->addWidget(weekAvgBtn);

    buttonLayoutBottom->addSpacing(10);

    // "一月数据"按钮
    QPushButton *monthDataBtn = new QPushButton("📅 一月数据");
    monthDataBtn->setObjectName("btnMonthData");
    monthDataBtn->setMinimumSize(120, 40);
    monthDataBtn->setStyleSheet(R"(
        QPushButton#btnMonthData {
            background-color: #7950f2;
            color: white;
            border: none;
            border-radius: 6px;
            font-weight: bold;
        }
        QPushButton#btnMonthData:hover {
            background-color: #6741d9;
        }
    )");
    connect(monthDataBtn, &QPushButton::clicked, this, &MainWindow::onShowMonthDataClicked);
    buttonLayoutBottom->addWidget(monthDataBtn);

    buttonLayoutBottom->addSpacing(10);

    // "月度均值"按钮
    QPushButton *monthBtn = new QPushButton("📅 月度均值");
    monthBtn->setObjectName("btnMonthAvg");
    monthBtn->setMinimumSize(120, 40);
    monthBtn->setStyleSheet(R"(
        QPushButton#btnMonthAvg {
            background-color: #ff922b;
            color: white;
            border: none;
            border-radius: 6px;
            font-weight: bold;
        }
        QPushButton#btnMonthAvg:hover {
            background-color: #e07a1f;
        }
    )");
    connect(monthBtn, &QPushButton::clicked, this, &MainWindow::onShowMonthAvgClicked);
    buttonLayoutBottom->addWidget(monthBtn);

    buttonLayoutBottom->addSpacing(10);

    QPushButton *allDataBtn = new QPushButton("📊 查看全部数据");
    allDataBtn->setObjectName("btnAllData");
    allDataBtn->setMinimumSize(140, 40);
    allDataBtn->setStyleSheet(R"(
        QPushButton#btnAllData {
            background-color: #6c5ce7;
            color: white;
            border: none;
            border-radius: 6px;
            font-weight: bold;
        }
        QPushButton#btnAllData:hover {
            background-color: #5b4cdb;
        }
    )");
    connect(allDataBtn, &QPushButton::clicked, this, &MainWindow::onShowAllDataClicked);
    buttonLayoutBottom->addWidget(allDataBtn);

    buttonLayoutBottom->addSpacing(10);

    QPushButton *closeBtn = new QPushButton("✕ 关闭");
    closeBtn->setObjectName("btnCloseHistory");
    closeBtn->setMinimumSize(120, 40);
    connect(closeBtn, &QPushButton::clicked, historyWindow, &QWidget::hide);
    buttonLayoutBottom->addWidget(closeBtn);
    mainLayout->addLayout(buttonLayoutBottom);

    // 默认隐藏
    historyWindow->hide();

    // 创建查看全部数据窗口
    createAllDataWindow();

    qDebug() << "✅ 历史数据窗口创建完成";
}

void MainWindow::showHistoryForSensor(int sensorIndex)
{
    if (sensorIndex < 0 || sensorIndex >= sensorInfoList.size()) {
        qDebug() << "❌ 无效的传感器索引:" << sensorIndex;
        return;
    }

    SensorType type = sensorInfoList[sensorIndex].type;
    QString sensorName = sensorInfoList[sensorIndex].name;

    // 记录当前传感器类型，供周/月按钮复用
    currentSensorType = type;

    // 显示窗口
    if (historyWindow) {
        historyWindow->show();
        historyWindow->raise();
        historyWindow->activateWindow();
    }

    // 查询并显示数据
    querySensorHistory(type);

    // 更新窗口标题
    if (historyWindow) {
        historyWindow->setWindowTitle(QString("📊 %1 历史数据").arg(sensorName));
    }

    qDebug() << "📊 显示" << sensorName << "历史数据";
}

void MainWindow::onHistoryButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int index = btn->property("sensorIndex").toInt();
    showHistoryForSensor(index);
}

void MainWindow::onShowHistoryClicked()
{
    if (historyWindow) {
        historyWindow->show();
        historyWindow->raise();
        historyWindow->activateWindow();
        // 默认显示温度数据
        showHistoryForSensor(0);
    }
}

void MainWindow::onShowWeekDataClicked()
{
    if (!historyWindow) return;

    // 确保窗口可见
    historyWindow->show();
    historyWindow->raise();
    historyWindow->activateWindow();

    // 使用当前传感器类型查询一周数据
    QString sensorName;
    for (const auto &info : sensorInfoList) {
        if (info.type == currentSensorType) {
            sensorName = info.name;
            break;
        }
    }

    queryWeekHistory(currentSensorType);

    // 更新窗口标题
    if (historyWindow) {
        historyWindow->setWindowTitle(QString("📊 %1 一周历史数据").arg(sensorName));
    }

    qDebug() << "📅 显示一周数据:" << sensorName;
}

void MainWindow::onShowMonthAvgClicked()
{
    if (!historyWindow) return;

    // 确保窗口可见
    historyWindow->show();
    historyWindow->raise();
    historyWindow->activateWindow();

    // 使用当前传感器类型查询月度均值
    QString sensorName;
    for (const auto &info : sensorInfoList) {
        if (info.type == currentSensorType) {
            sensorName = info.name;
            break;
        }
    }

    queryMonthAvgHistory(currentSensorType);

    // 更新窗口标题
    if (historyWindow) {
        historyWindow->setWindowTitle(QString("📊 %1 月度日均值").arg(sensorName));
    }

    qDebug() << "📅 显示月度均值:" << sensorName;
}

void MainWindow::onShowWeekAvgClicked()
{
    if (!historyWindow) return;

    historyWindow->show();
    historyWindow->raise();
    historyWindow->activateWindow();

    QString sensorName;
    for (const auto &info : sensorInfoList) {
        if (info.type == currentSensorType) {
            sensorName = info.name;
            break;
        }
    }

    queryWeekAvgHistory(currentSensorType);

    if (historyWindow) {
        historyWindow->setWindowTitle(QString("📊 %1 一周日均值").arg(sensorName));
    }

    qDebug() << "📅 显示一周日均值:" << sensorName;
}

void MainWindow::onShowMonthDataClicked()
{
    if (!historyWindow) return;

    historyWindow->show();
    historyWindow->raise();
    historyWindow->activateWindow();

    QString sensorName;
    for (const auto &info : sensorInfoList) {
        if (info.type == currentSensorType) {
            sensorName = info.name;
            break;
        }
    }

    queryMonthHistory(currentSensorType);

    if (historyWindow) {
        historyWindow->setWindowTitle(QString("📊 %1 一月历史数据").arg(sensorName));
    }

    qDebug() << "📅 显示一月全部数据:" << sensorName;
}

// ==================== 连接相关 ====================

void MainWindow::onConnected()
{
    updateConnectionStatus(true);
    qDebug() << "✅ 已连接到服务器";

    // ========== 新增：连接成功后注册设备并发送阈值 ==========
    QTimer::singleShot(500, this, [this]() {
        sendDeviceRegister();
        // 延迟一下再发送阈值
        QTimer::singleShot(300, this, [this]() {
            sendThresholdSetting();
        });
    });
}

void MainWindow::onDisconnected()
{
    updateConnectionStatus(false);
    qDebug() << "❌ 已断开连接";
}

void MainWindow::onError(const QString &error)
{
    ui->labelStatus->setText("❌ 连接错误");
    ui->labelStatus->setStyleSheet("color: #ff4444;");
    QMessageBox::warning(this, "错误", "连接错误: " + error);
}

// ==================== 设备控制 ====================

void MainWindow::onLED2SetClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20011;
    cmd["id"] = 1;
    cmd["light"] = ui->spinLED2->value();
    sendCommand(cmd);
    statusBar()->showMessage(QString("LED2 亮度: %1").arg(ui->spinLED2->value()), 2000);
}

void MainWindow::onRGBSetClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20012;
    cmd["red"] = ui->spinRed->value();
    cmd["green"] = ui->spinGreen->value();
    cmd["blue"] = ui->spinBlue->value();
    sendCommand(cmd);
    statusBar()->showMessage("RGB 已设置", 2000);
}

void MainWindow::onFanOnClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20013;
    cmd["sw"] = true;
    sendCommand(cmd);
    statusBar()->showMessage("风扇已开启", 2000);
}

void MainWindow::onFanOffClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20013;
    cmd["sw"] = false;
    sendCommand(cmd);
    statusBar()->showMessage("风扇已关闭", 2000);
}

void MainWindow::onAlarmOnClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20014;
    cmd["sw"] = true;
    sendCommand(cmd);
    statusBar()->showMessage("报警器已开启", 2000);
}

void MainWindow::onAlarmOffClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20014;
    cmd["sw"] = false;
    sendCommand(cmd);
    statusBar()->showMessage("报警器已关闭", 2000);
}

void MainWindow::onAlarmLightOnClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20015;
    cmd["sw"] = true;
    sendCommand(cmd);
    statusBar()->showMessage("报警灯已开启", 2000);
}

void MainWindow::onAlarmLightOffClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20015;
    cmd["sw"] = false;
    sendCommand(cmd);
    statusBar()->showMessage("报警灯已关闭", 2000);
}

// ==================== 传感器触发（手动） ====================

void MainWindow::onCo2Clicked()
{
    QJsonObject cmd;
    cmd["type"] = 20001;
    sendCommand(cmd);
    statusBar()->showMessage("触发: CO2 上报", 2000);
}

void MainWindow::onTemperatureClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20002;
    sendCommand(cmd);
    statusBar()->showMessage("触发: 温度上报", 2000);
}

void MainWindow::onFireClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20003;
    sendCommand(cmd);
    statusBar()->showMessage("触发: 火光上报", 2000);
}

void MainWindow::onPM25Clicked()
{
    QJsonObject cmd;
    cmd["type"] = 20004;
    sendCommand(cmd);
    statusBar()->showMessage("触发: PM2.5 上报", 2000);
}

void MainWindow::onMethaneClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20005;
    sendCommand(cmd);
    statusBar()->showMessage("触发: 甲烷上报", 2000);
}

void MainWindow::onLightClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20006;
    sendCommand(cmd);
    statusBar()->showMessage("触发: 光照上报", 2000);
}

void MainWindow::onSmokeClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20007;
    sendCommand(cmd);
    statusBar()->showMessage("触发: 烟雾上报", 2000);
}

void MainWindow::onHumidityClicked()
{
    QJsonObject cmd;
    cmd["type"] = 20008;
    sendCommand(cmd);
    statusBar()->showMessage("触发: 湿度上报", 2000);
}

// ==================== 接收数据 ====================

void MainWindow::onDataReceived(const QJsonObject &data)
{
    int type = data.value("type").toInt();

    // ===== 处理服务器对阈值设置的确认 (30002) =====
    if (type == 30002) {
        int roomId = data.value("room_id").toInt();
        bool success = data.value("success").toBool(false);
        if (success) {
            updateThresholdStatus(QString("房间 %1 阈值设置成功").arg(roomId), true);
            QMessageBox::information(this, "成功", QString("房间 %1 的联动阈值已成功设置到服务器").arg(roomId));
        } else {
            updateThresholdStatus("阈值设置失败", false);
            QMessageBox::warning(this, "错误", "阈值设置失败");
        }
        return;
    }

    // ===== 处理传感器数据 (10001~10008) =====
    QVariant value = data.value("value");

    qDebug() << "========================================";
    qDebug() << "📥 收到数据: type=" << type << ", value=" << value;
    qDebug() << "📥 完整数据:" << data;
    qDebug() << "========================================";

    bool updated = false;

    switch(type) {
    case 10001:
        sensorData.co2 = value.toDouble();
        updated = true;
        qDebug() << "  ✅ 更新 CO2:" << sensorData.co2;
        break;
    case 10002:
        sensorData.temperature = value.toDouble();
        updated = true;
        qDebug() << "  ✅ 更新 温度:" << sensorData.temperature;
        break;
    case 10003:
        sensorData.fire = value.toBool() ? 1.0 : 0.0;
        updated = true;
        qDebug() << "  ✅ 更新 火光:" << (value.toBool() ? "检测到" : "正常");
        break;
    case 10004:
        sensorData.pm25 = value.toDouble();
        updated = true;
        qDebug() << "  ✅ 更新 PM2.5:" << sensorData.pm25;
        break;
    case 10005:
        sensorData.methane = value.toBool() ? 1.0 : 0.0;
        updated = true;
        qDebug() << "  ✅ 更新 甲烷:" << (value.toBool() ? "异常" : "正常");
        break;
    case 10006:
        sensorData.light = value.toDouble();
        updated = true;
        qDebug() << "  ✅ 更新 光照:" << sensorData.light;
        break;
    case 10007:
        sensorData.smoke = value.toBool() ? 1.0 : 0.0;
        updated = true;
        qDebug() << "  ✅ 更新 烟雾:" << (value.toBool() ? "异常" : "正常");
        break;
    case 10008:
        sensorData.humidity = value.toDouble();
        updated = true;
        qDebug() << "  ✅ 更新 湿度:" << sensorData.humidity;
        break;
    default:
        qDebug() << "  ⚠️ 未知类型:" << type;
        statusBar()->showMessage(QString("收到未知数据: type=%1").arg(type), 2000);
        return;
    }

    if (updated) {
        updateSensorDisplay();
        statusBar()->showMessage("✅ 传感器数据已更新", 2000);
    }
}

// ==================== 查看全部数据窗口 ====================

void MainWindow::createAllDataWindow()
{
    allDataWindow = new QWidget();
    allDataWindow->setWindowTitle("📊 全部传感器历史数据");
    allDataWindow->setMinimumSize(1200, 900);
    allDataWindow->setStyleSheet(R"(
        QWidget {
            background-color: #f5f5f5;
        }
        QGroupBox {
            background-color: #ffffff;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            margin-top: 12px;
            padding: 8px;
            font-weight: bold;
        }
        QGroupBox::title {
            color: #333333;
            font-size: 12px;
            font-weight: bold;
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 8px;
        }
        QPushButton#btnCloseAllData {
            background-color: #e9ecef;
            color: #333333;
            border: none;
            border-radius: 6px;
            padding: 10px 24px;
            font-weight: bold;
        }
        QPushButton#btnCloseAllData:hover {
            background-color: #dee2e6;
        }
        QChartView {
            background-color: #ffffff;
            border-radius: 8px;
        }
    )");

    QVBoxLayout *mainLayout = new QVBoxLayout(allDataWindow);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(16, 16, 16, 16);

    // 创建8个图表的网格布局
    QGridLayout *chartsLayout = new QGridLayout();
    chartsLayout->setSpacing(12);

    QStringList sensorNames = {"温度", "湿度", "CO₂", "PM2.5", "甲烷", "光照", "烟雾", "火光"};
    QStringList units = {"°C", "%", "ppm", "µg/m³", "%", "lux", "%", ""};
    QList<QColor> colors = {
        QColor("#e94560"), QColor("#4dabf7"), QColor("#20c997"), QColor("#ff922b"),
        QColor("#fcc419"), QColor("#f59f00"), QColor("#868e96"), QColor("#e03131")
    };

    for (int i = 0; i < 8; i++) {
        QGroupBox *chartGroup = new QGroupBox(sensorNames[i]);
        QVBoxLayout *chartLayout = new QVBoxLayout(chartGroup);

        QChart *chart = new QChart();
        chart->setTitle("");
        chart->setAnimationOptions(QChart::NoAnimation);
        chart->setTheme(QChart::ChartThemeLight);
        chart->legend()->setVisible(true);
        chart->legend()->setAlignment(Qt::AlignBottom);

        QLineSeries *lineSeries = new QLineSeries();
        lineSeries->setName(sensorNames[i]);
        lineSeries->setPointsVisible(true);
        lineSeries->setColor(colors[i]);
        lineSeries->setPointLabelsVisible(false);
        chart->addSeries(lineSeries);

        QDateTimeAxis *dateAxis = new QDateTimeAxis();
        dateAxis->setTitleText("时间");
        dateAxis->setFormat("hh:mm");
        dateAxis->setGridLineVisible(true);
        chart->addAxis(dateAxis, Qt::AlignBottom);
        lineSeries->attachAxis(dateAxis);

        QValueAxis *valueAxis = new QValueAxis();
        valueAxis->setTitleText("数值");
        valueAxis->setLabelFormat("%.1f");
        valueAxis->setGridLineVisible(true);
        chart->addAxis(valueAxis, Qt::AlignLeft);
        lineSeries->attachAxis(valueAxis);

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
        chartView->setMinimumHeight(180);
        chartView->setMinimumWidth(280);

        chartLayout->addWidget(chartView);
        chartsLayout->addWidget(chartGroup, i / 4, i % 4);

        allChartViews.append(chartView);
        allSeries.append(lineSeries);
        allAxisX.append(dateAxis);
        allAxisY.append(valueAxis);
    }

    mainLayout->addLayout(chartsLayout);

    // 关闭按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    QPushButton *closeBtn = new QPushButton("✕ 关闭");
    closeBtn->setObjectName("btnCloseAllData");
    closeBtn->setMinimumSize(140, 40);
    connect(closeBtn, &QPushButton::clicked, allDataWindow, &QWidget::hide);
    buttonLayout->addWidget(closeBtn);
    mainLayout->addLayout(buttonLayout);

    allDataWindow->hide();

    qDebug() << "✅ 全部数据窗口创建完成";
}

void MainWindow::queryAllSensorHistory()
{
    for (int i = 0; i < sensorInfoList.size(); i++) {
        if (i >= allSeries.size()) break;

        SensorType type = sensorInfoList[i].type;
        QString tableName = sensorInfoList[i].tableName;
        QString unit = sensorInfoList[i].unit;

        if (!dbConnected || !db.isOpen()) {
            if (!initDatabase()) continue;
        }

        QStringList tables = db.tables();
        if (!tables.contains(tableName, Qt::CaseInsensitive)) {
            continue;
        }

        QString queryStr = QString("SELECT * FROM `%1` ORDER BY id DESC LIMIT 30").arg(tableName);
        QSqlQuery query;

        if (!query.exec(queryStr)) {
            continue;
        }

        QLineSeries *lineSeries = allSeries[i];
        lineSeries->clear();

        QList<QPair<QDateTime, double>> dataPoints;

        QString valueField = "value";
        QString timeField = "timestamp";
        QSqlRecord record = query.record();

        if (record.indexOf("value") == -1) {
            if (record.indexOf("data") != -1) valueField = "data";
            else if (record.indexOf("val") != -1) valueField = "val";
        }
        if (record.indexOf("timestamp") == -1) {
            if (record.indexOf("time") != -1) timeField = "time";
            else if (record.indexOf("datetime") != -1) timeField = "datetime";
            else if (record.indexOf("create_time") != -1) timeField = "create_time";
        }

        queryStr = QString("SELECT * FROM `%1` ORDER BY id DESC LIMIT 30").arg(tableName);
        if (!query.exec(queryStr)) {
            continue;
        }

        while (query.next()) {
            QDateTime timestamp;
            QVariant timeVar = query.value(timeField);
            if (timeVar.canConvert<QDateTime>()) {
                timestamp = timeVar.toDateTime();
            } else if (timeVar.typeId() == QMetaType::QString) {
                timestamp = QDateTime::fromString(timeVar.toString(), "yyyy-MM-dd hh:mm:ss");
                if (!timestamp.isValid()) {
                    timestamp = QDateTime::fromString(timeVar.toString(), "yyyy-MM-dd HH:mm:ss");
                }
            } else if (timeVar.typeId() == QMetaType::Int || timeVar.typeId() == QMetaType::LongLong) {
                qint64 ts = timeVar.toLongLong();
                if (ts > 10000000000) {
                    timestamp = QDateTime::fromMSecsSinceEpoch(ts);
                } else {
                    timestamp = QDateTime::fromSecsSinceEpoch(ts);
                }
            }

            if (!timestamp.isValid()) {
                timestamp = QDateTime::currentDateTime().addSecs(-dataPoints.size() * 10);
            }

            double value = query.value(valueField).toDouble();
            dataPoints.prepend({timestamp, value});
        }

        if (dataPoints.size() < 2) {
            QRandomGenerator *gen = QRandomGenerator::global();
            dataPoints.clear();
            for (int j = 0; j < 20; j++) {
                double val;
                switch(type) {
                case SENSOR_TEMPERATURE:
                    val = 20.0 + (gen->generateDouble() * 3.0);
                    break;
                case SENSOR_HUMIDITY:
                    val = 40.0 + (gen->generateDouble() * 4.0);
                    break;
                case SENSOR_CO2:
                    val = 400 + gen->bounded(200);
                    break;
                case SENSOR_PM25:
                    val = 10 + gen->bounded(100);
                    break;
                case SENSOR_METHANE:
                    val = gen->generateDouble();
                    break;
                case SENSOR_LIGHT:
                    val = 100 + gen->bounded(500);
                    break;
                case SENSOR_SMOKE:
                    val = gen->generateDouble();
                    break;
                case SENSOR_FIRE:
                    val = gen->bounded(2);
                    break;
                default:
                    val = gen->bounded(100);
                }
                QDateTime dt = QDateTime::currentDateTime().addSecs(-(20 - j) * 10);
                dataPoints.append({dt, val});
            }
        }

        for (const auto &point : dataPoints) {
            lineSeries->append(point.first.toMSecsSinceEpoch(), point.second);
        }

        QValueAxis *axisY = allAxisY[i];
        QString labelFormat = unit.isEmpty() ? "%.1f" : "%.1f " + unit;
        axisY->setLabelFormat(labelFormat);

        if (lineSeries->count() > 0) {
            double minY = lineSeries->at(0).y();
            double maxY = lineSeries->at(0).y();
            for (int j = 0; j < lineSeries->count(); j++) {
                double y = lineSeries->at(j).y();
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
            double padding = (maxY - minY) * 0.1;
            if (padding < 1) padding = 1;
            axisY->setRange(minY - padding, maxY + padding);
        }

        QDateTimeAxis *axisX = allAxisX[i];
        if (lineSeries->count() > 0) {
            qreal minX = lineSeries->at(0).x();
            qreal maxX = lineSeries->at(0).x();
            for (int j = 0; j < lineSeries->count(); j++) {
                qreal x = lineSeries->at(j).x();
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
            }
            axisX->setRange(QDateTime::fromMSecsSinceEpoch(minX), QDateTime::fromMSecsSinceEpoch(maxX));
        }
    }

    qDebug() << "✅ 全部传感器历史数据加载完成";
}

void MainWindow::onShowAllDataClicked()
{
    if (!allDataWindow) {
        createAllDataWindow();
    }

    queryAllSensorHistory();

    allDataWindow->show();
    allDataWindow->raise();
    allDataWindow->activateWindow();

    qDebug() << "📊 显示全部传感器历史数据";
}

// ==================== 阈值设置相关 ====================

void MainWindow::loadDefaultThresholds()
{
    thresholdConfig.roomId = 1;
    thresholdConfig.co2Max = 1000;
    thresholdConfig.pm25Max = 50;
    thresholdConfig.tempMax = 30.0;
    thresholdConfig.tempMin = 18.0;
    thresholdConfig.methaneEnable = true;
    thresholdConfig.cooldownSeconds = 30;
    thresholdConfig.fanCooldownSeconds = 30;

    updateThresholdUI();
    updateThresholdStatus("已加载默认值", true);
    qDebug() << "📋 加载默认阈值配置";
}

void MainWindow::updateThresholdUI()
{
    // 从配置更新UI控件
    ui->spinRoomId->setValue(thresholdConfig.roomId);
    ui->spinCo2Max->setValue(thresholdConfig.co2Max);
    ui->spinPM25Max->setValue(thresholdConfig.pm25Max);
    ui->spinTempMax->setValue(thresholdConfig.tempMax);
    ui->spinTempMin->setValue(thresholdConfig.tempMin);
    ui->checkMethaneEnable->setChecked(thresholdConfig.methaneEnable);
    ui->spinCooldown->setValue(thresholdConfig.cooldownSeconds);
    if (spinFanCooldown)
        spinFanCooldown->setValue(thresholdConfig.fanCooldownSeconds);
}

void MainWindow::updateThresholdStatus(const QString &status, bool success)
{
    QString style = success ? "color: #20c997; font-size: 13px;"
                            : "color: #ff4444; font-size: 13px;";
    ui->labelThresholdStatus->setText(success ? "✅ " + status : "❌ " + status);
    ui->labelThresholdStatus->setStyleSheet(style);
}

void MainWindow::onLoadDefaultClicked()
{
    loadDefaultThresholds();
    QMessageBox::information(this, "恢复默认", "阈值已恢复为默认值");
}

void MainWindow::onApplyThresholdClicked()
{
    if (!tcpClient->isConnected()) {
        QMessageBox::warning(this, "错误", "未连接到服务器，请先连接");
        return;
    }

    // 从UI读取阈值配置
    thresholdConfig.roomId = ui->spinRoomId->value();
    thresholdConfig.co2Max = ui->spinCo2Max->value();
    thresholdConfig.pm25Max = ui->spinPM25Max->value();
    thresholdConfig.tempMax = ui->spinTempMax->value();
    thresholdConfig.tempMin = ui->spinTempMin->value();
    thresholdConfig.methaneEnable = ui->checkMethaneEnable->isChecked();
    thresholdConfig.cooldownSeconds = ui->spinCooldown->value();
    thresholdConfig.fanCooldownSeconds = spinFanCooldown ? spinFanCooldown->value() : 30;

    qDebug() << "📊 发送阈值配置到服务器:";
    qDebug() << "  roomId:" << thresholdConfig.roomId;
    qDebug() << "  co2Max:" << thresholdConfig.co2Max;
    qDebug() << "  pm25Max:" << thresholdConfig.pm25Max;
    qDebug() << "  tempMax:" << thresholdConfig.tempMax;
    qDebug() << "  tempMin:" << thresholdConfig.tempMin;
    qDebug() << "  methaneEnable:" << thresholdConfig.methaneEnable;
    qDebug() << "  cooldownSeconds:" << thresholdConfig.cooldownSeconds;
    qDebug() << "  fanCooldownSeconds:" << thresholdConfig.fanCooldownSeconds;

    sendThresholdSetting();
}

void MainWindow::sendThresholdSetting()
{
    if (!tcpClient->isConnected()) {
        updateThresholdStatus("未连接", false);
        return;
    }

    QJsonObject th;
    th["type"] = 30001;
    th["room_id"] = thresholdConfig.roomId;
    th["co2_max"] = thresholdConfig.co2Max;
    th["pm25_max"] = thresholdConfig.pm25Max;
    th["temp_max"] = thresholdConfig.tempMax;
    th["temp_min"] = thresholdConfig.tempMin;
    th["methane_enable"] = thresholdConfig.methaneEnable;
    th["cooldown_seconds"] = thresholdConfig.cooldownSeconds;
    th["fan_cooldown_seconds"] = thresholdConfig.fanCooldownSeconds;

    tcpClient->sendCommand(th);
    updateThresholdStatus("已发送，等待服务器确认...", true);
    qDebug() << "📤 发送阈值设置:" << th;
}

void MainWindow::sendDeviceRegister()
{
    if (!tcpClient->isConnected()) {
        qDebug() << "⚠️ 未连接，无法注册设备";
        return;
    }

    QJsonObject reg;
    reg["type"] = 1;
    reg["room_id"] = thresholdConfig.roomId;
    reg["is_gateway"] = false;   // 客户端不是网关，设为 false 才能收到传感器广播
    reg["device_id"] = QString("client_%1").arg(thresholdConfig.roomId);

    tcpClient->sendCommand(reg);
    qDebug() << "📱 发送设备注册:" << reg;
}

void MainWindow::onServerCommandReceived(const QJsonObject &cmd)
{
    // 20050 广播中的 action 字段包含告警原因文本（如 "CO₂超标: 1100 ppm > 1000 ppm"）
    QString alertReason = cmd.value("action").toString();
    double value = cmd.value("value").toDouble();
    int roomId = cmd.value("room_id").toInt();
    QString timestamp = cmd.value("timestamp").toString();

    qDebug() << "========================================";
    qDebug() << "📟 收到服务器告警通知 (type=20050):";
    qDebug() << "  告警原因: " << alertReason;
    qDebug() << "  超标数值: " << value;
    qDebug() << "  房间: " << roomId;
    qDebug() << "  时间: " << timestamp;
    qDebug() << "========================================";

    // 状态栏显示告警原因
    statusBar()->showMessage(QString("🚨 %1").arg(alertReason), 8000);

    // 根据告警原因判断严重程度，紧急情况弹窗提醒
    // 服务器已经直接向网关发送了设备控制指令 (20013/20014/20015)
    // 客户端只需显示通知，不需要重复发送控制命令
    if (alertReason.contains("火焰") || alertReason.contains("烟雾")) {
        QMessageBox::warning(this, "🚨 紧急安全警报",
                             QString("检测到安全隐患！\n服务器已自动开启报警器和报警灯。\n\n"
                                     "原因: %1\n时间: %2\n\n"
                                     "报警灯和报警器可通过面板按钮手动关闭。")
                                 .arg(alertReason).arg(timestamp));
    }
    else if (alertReason.contains("甲烷")) {
        QMessageBox::warning(this, "⚠️ 燃气泄漏警告",
                             QString("检测到甲烷泄漏！\n服务器已自动开启报警器和报警灯。\n\n"
                                     "原因: %1\n时间: %2\n\n"
                                     "请立即检查燃气管道！报警灯和报警器可通过面板按钮手动关闭。")
                                 .arg(alertReason).arg(timestamp));
    }
    else {
        // CO2/PM2.5/温度超标 → 弹信息提示
        statusBar()->showMessage(QString("🔔 联动触发: %1 (风扇已自动开启)").arg(alertReason), 8000);
    }

    // 发送确认回执给服务器
    QJsonObject result;
    result["type"] = 20051;
    result["action"] = alertReason;
    result["success"] = true;
    result["message"] = "客户端已收到告警通知";
    result["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    if (tcpClient->isConnected()) {
        tcpClient->sendCommand(result);
        qDebug() << "📤 发送告警回执 (type=20051):" << result;
    }
}