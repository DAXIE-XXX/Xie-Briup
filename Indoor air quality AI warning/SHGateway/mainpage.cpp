//===================================================================
//  mainpage.cpp
//  完整实现：串口打开/关闭、设备控制、传感器读取、UI 更新
//===================================================================
#include "mainpage.h"
#include "ui_mainpage.h"
#include "serialcontext.h"
#include "netcontext.h"
#include <QSerialPort>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>

MainPage::MainPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainPage)
    , serial(SerialContext::getObject())
{
    ui->setupUi(this);

    /* ---------- 1) 填充下拉框 ---------- */
    fillPortList();
    fillBaudRateList();

    /* ---------- 2) 关联信号‑槽 ---------- */
    // ---------- 串口 ----------
    connect(ui->btnOpenSerial,  &QPushButton::clicked,
            this,                &MainPage::onOpenSerialClicked);
    connect(ui->btnCloseSerial, &QPushButton::clicked,
            this,                &MainPage::onCloseSerialClicked);

    // ---------- 设备控制 ----------
    connect(ui->btnAlarmOn,       &QPushButton::clicked,
            this,                  &MainPage::onAlarmOnClicked);
    connect(ui->btnAlarmOff,      &QPushButton::clicked,
            this,                  &MainPage::onAlarmOffClicked);
    connect(ui->btnAlarmLightOn,  &QPushButton::clicked,
            this,                  &MainPage::onAlarmLightOnClicked);
    connect(ui->btnAlarmLightOff, &QPushButton::clicked,
            this,                  &MainPage::onAlarmLightOffClicked);
    connect(ui->btnLED2On,        &QPushButton::clicked,
            this,                  &MainPage::onLED2OnClicked);
    connect(ui->btnLED2Off,       &QPushButton::clicked,
            this,                  &MainPage::onLED2OffClicked);
    connect(ui->btnFanOn,         &QPushButton::clicked,
            this,                  &MainPage::onFanOnClicked);
    connect(ui->btnFanOff,        &QPushButton::clicked,
            this,                  &MainPage::onFanOffClicked);

    // ---------- 传感器检测 ----------
    connect(ui->btnPM25,          &QPushButton::clicked,
            this,                  &MainPage::onPM25Clicked);
    connect(ui->btnSmoke,         &QPushButton::clicked,
            this,                  &MainPage::onSmokeClicked);
    connect(ui->btnMethane,       &QPushButton::clicked,
            this,                  &MainPage::onMethaneClicked);
    connect(ui->btnLight,         &QPushButton::clicked,
            this,                  &MainPage::onLightClicked);
    connect(ui->btnCo2,           &QPushButton::clicked,
            this,                  &MainPage::onCo2Clicked);
    connect(ui->btnFire,          &QPushButton::clicked,
            this,                  &MainPage::onFireClicked);
    connect(ui->btnTemperature,   &QPushButton::clicked,
            this,                  &MainPage::onTemperatureClicked);
    connect(ui->btnHumidity,      &QPushButton::clicked,
            this,                  &MainPage::onHumidityClicked);

    // ---------- 传感器数据更新 ----------
    connect(serial, &SerialContext::getPM25Sig,          this, &MainPage::updatePM25);
    connect(serial, &SerialContext::getSmokeSig,        this, &MainPage::updateSmoke);
    connect(serial, &SerialContext::getMethaneSig,      this, &MainPage::updateMethane);
    connect(serial, &SerialContext::getLightSig,        this, &MainPage::updateLight);
    connect(serial, &SerialContext::getCo2Sig,          this, &MainPage::updateCo2);
    connect(serial, &SerialContext::getFireSig,         this, &MainPage::updateFire);
    connect(serial, &SerialContext::getTemperatureSig,  this, &MainPage::updateTemperature);
    connect(serial, &SerialContext::getHumiditySig,    this, &MainPage::updateHumidity);

    // ---------- 服务器连接 ----------
    connect(ui->btnConnectServer, &QPushButton::clicked,
            this, &MainPage::onConnectServerClicked);

    // ---------- 读取 INI 配置文件自动连接 ----------
    QSettings settings("gateway_config.ini", QSettings::IniFormat);
    if (settings.contains("server/ip")) {
        QString savedIp = settings.value("server/ip").toString();
        quint16 savedPort = settings.value("server/port", 10086).toUInt();
        bool autoConnect = settings.value("server/auto_connect", false).toBool();
        ui->editIP->setText(savedIp);
        ui->editPort->setText(QString::number(savedPort));
        if (autoConnect) {
            // 延迟 500ms 后自动连接，给 UI 初始化和事件循环足够时间
            QTimer::singleShot(500, this, &MainPage::onConnectServerClicked);
        }
    }

    startTimer(10000);
}

/*---------------------------------------------------------------*/
MainPage::~MainPage()
{
    delete ui;
}

/*---------------------------------------------------------------*/
void MainPage::fillPortList()
{
    ui->comboPort->clear();
    ui->comboPort->addItems(serial->getPortNames());
    if (ui->comboPort->count() > 0)
        ui->comboPort->setCurrentIndex(0);
}

/*---------------------------------------------------------------*/
void MainPage::fillBaudRateList()
{
    const QList<quint32> baud = {9600,19200,38400,57600,115200,230400};
    ui->comboBaud->clear();
    for (quint32 b : baud)
        ui->comboBaud->addItem(QString::number(b), b);
    ui->comboBaud->setCurrentText(QString::number(57600));  // 硬件默认波特率
}

/*---------------------------------------------------------------*/
bool MainPage::tryOpenSerial()
{
    if (serial->openSerial(ui->comboPort->currentText(),
                           ui->comboBaud->currentData().toUInt())) {
        QMessageBox::information(this, tr("串口"), tr("串口打开成功"));
        return true;
    } else {
        QMessageBox::critical(this, tr("串口"), tr("打开串口失败，请检查端口或波特率"));
        return false;
    }
}

/*---------------------------------------------------------------*/
/*  私有帮助函数：检查服务器是否已连接，未连接弹一次警告后返回 false  */
bool MainPage::warnIfNotConnected()
{
    if (NetContext::getObject()->isConnected())
        return true;

    QMessageBox::warning(this, tr("服务器未连接"),
                         tr("请先连接服务器，数据将不会上报"));
    return false;
}

/*---------------------------------------------------------------*/
/*   串口打开 / 关闭槽   */
void MainPage::onOpenSerialClicked()   { tryOpenSerial(); }
void MainPage::onCloseSerialClicked()
{
    if (serial->serial->isOpen()) {
        serial->serial->close();
        QMessageBox::information(this, tr("串口"), tr("串口已关闭"));
    } else {
        QMessageBox::information(this, tr("串口"), tr("串口本来就是关闭状态"));
    }
}

/*---------------------------------------------------------------*/
/*   设备控制槽（保持原有 8 个）   */
void MainPage::onAlarmOnClicked()        { serial->setAlarm(true);  }
void MainPage::onAlarmOffClicked()       { serial->setAlarm(false); }
void MainPage::onAlarmLightOnClicked()   { serial->setAlarmLight(true);  }
void MainPage::onAlarmLightOffClicked()  { serial->setAlarmLight(false); }
void MainPage::onLED2OnClicked()         { serial->setLED2(0xFF); }
void MainPage::onLED2OffClicked()        { serial->setLED2(0x00); }
void MainPage::onFanOnClicked()          { serial->setFun(true);  }
void MainPage::onFanOffClicked()         { serial->setFun(false); }

/*---------------------------------------------------------------*/
/*   传感器请求槽（8 个）   */
void MainPage::onPM25Clicked()            { if (!serial->serial->isOpen()) tryOpenSerial(); serial->getPM25(); }
void MainPage::onSmokeClicked()           { if (!serial->serial->isOpen()) tryOpenSerial(); serial->getSmoke(); }
void MainPage::onMethaneClicked()         { if (!serial->serial->isOpen()) tryOpenSerial(); serial->getMethane(); }
void MainPage::onLightClicked()           { if (!serial->serial->isOpen()) tryOpenSerial(); serial->getLight(); }
void MainPage::onCo2Clicked()             { if (!serial->serial->isOpen()) tryOpenSerial(); serial->getCo2(); }
void MainPage::onFireClicked()            { if (!serial->serial->isOpen()) tryOpenSerial(); serial->getFire(); }
void MainPage::onTemperatureClicked()    { if (!serial->serial->isOpen()) tryOpenSerial(); serial->getTemperature(); }
void MainPage::onHumidityClicked()       { if (!serial->serial->isOpen()) tryOpenSerial(); serial->getHumidity(); }

/*---------------------------------------------------------------*/
/*   传感器数据更新槽（每一个都先检查网络）   */
void MainPage::updatePM25(int value)
{
    if (!warnIfNotConnected()) return;
    NetContext::getObject()->pushPM25(value);
    ui->labelPM25->setText(QString::number(value));
}
void MainPage::updateSmoke(bool value)
{
    if (!warnIfNotConnected()) return;
    NetContext::getObject()->pushSmoke(value);
    ui->labelSmoke->setText(value ? tr("异常") : tr("正常"));
}
void MainPage::updateMethane(bool value)
{
    if (!warnIfNotConnected()) return;
    NetContext::getObject()->pushMethane(value);
    ui->labelMethane->setText(value ? tr("异常") : tr("正常"));
}
void MainPage::updateLight(int value)
{
    if (!warnIfNotConnected()) return;
    NetContext::getObject()->pushLight(value);
    ui->labelLight->setText(QString::number(value));
}
void MainPage::updateCo2(int value)
{
    if (!warnIfNotConnected()) return;
    NetContext::getObject()->pushCo2(value);
    ui->labelCo2->setText(QString::number(value));
}
void MainPage::updateFire(bool value)
{
    if (!warnIfNotConnected()) return;
    NetContext::getObject()->pushFire(value);
    ui->labelFire->setText(value ? tr("检测到火光") : tr("正常"));
}
void MainPage::updateTemperature(double value)
{
    if (!warnIfNotConnected()) return;
    NetContext::getObject()->pushTe1(value);
    ui->labelTemperature->setText(QString::number(value,'f',2) + tr(" °C"));
}
void MainPage::updateHumidity(double value)
{
    if (!warnIfNotConnected()) return;
    NetContext::getObject()->pushHumidity(value);
    ui->labelHumidity->setText(QString::number(value,'f',2) + tr(" %"));
}

/*---------------------------------------------------------------*/
/*   服务器连接槽   */
void MainPage::onConnectServerClicked()
{
    QString ip   = ui->editIP->text().trimmed();
    QString portStr = ui->editPort->text().trimmed();

    if (ip.isEmpty() || portStr.isEmpty()) {
        QMessageBox::warning(this, tr("服务器连接"),
                             tr("IP 地址和端口不能为空"));
        return;
    }
    bool ok = false;
    quint16 port = portStr.toUShort(&ok);
    if (!ok || port == 0) {
        QMessageBox::warning(this, tr("服务器连接"),
                             tr("端口必须是 1~65535 之间的数字"));
        return;
    }

    bool connected = NetContext::getObject()->connectToServer(ip, port);
    if (connected) {
        // 向服务器注册网关设备（默认房间号 1），使服务器建立联动映射
        NetContext::getObject()->registerToServer(1);
        // 保存连接信息到 INI，方便下次自动连接
        QSettings settings("gateway_config.ini", QSettings::IniFormat);
        settings.setValue("server/ip", ip);
        settings.setValue("server/port", port);
        settings.setValue("server/auto_connect", true);
        // 启用断线自动重连
        NetContext::getObject()->setAutoReconnect(true);
        QMessageBox::information(this, tr("服务器连接"),
                                 tr("成功连接到 %1:%2").arg(ip).arg(port));
    } else {
        QMessageBox::critical(this, tr("服务器连接"),
                              tr("连接失败，请检查 IP、端口以及服务器状态"));
    }
}

void MainPage::timerEvent(QTimerEvent *e)
{
    SerialContext::getObject()
        ->getCo2();
    SerialContext::getObject()
        ->getTemperature();
    SerialContext::getObject()
        ->getFire();
    SerialContext::getObject()
        ->getHumidity();
    SerialContext::getObject()
        ->getLight();
    SerialContext::getObject()
        ->getMethane();
    SerialContext::getObject()
        ->getSmoke();
    SerialContext::getObject()
        ->getPM25();
}