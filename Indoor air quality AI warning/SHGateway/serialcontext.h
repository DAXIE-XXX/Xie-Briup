#ifndef SERIALCONTEXT_H
#define SERIALCONTEXT_H

#include <QObject>
class QSerialPort;
class MainPage;
class SerialContext : public QObject
{
    Q_OBJECT
protected:
    explicit SerialContext(QObject *parent = nullptr);

public:
    static SerialContext *getObject();
    //获取系统可用串口
    QStringList getPortNames();
    //打开串口
    bool openSerial(QString name, quint16 baudrate);
    //发送控制指令
    void setLED2(int light);
    void setRGB(int r, int g, int b);
    void setFun(bool sw);
    void setAlarmLight(bool sw);//报警灯
    void setAlarm(bool sw);//报警器
    //发送数据请求 - 传感器
    void getPM25();      // PM2.5
    void getSmoke();     // 烟雾
    void getMethane();   // 甲烷
    void getLight();     // 光照
    void getCo2();       // CO2
    void getFire();      // 火光
    void getTemperature();// 温度1
    void getHumidity();  // 湿度1

signals:
    void getPM25Sig(int value);
    void getSmokeSig(bool value);
    void getMethaneSig(bool value);
    void getLightSig(int value);
    void getCo2Sig(int value);
    void getFireSig(bool value);
    void getTemperatureSig(double value);
    void getHumiditySig(double value);

protected slots:
    void readyReadSlot();

protected:
    void handleData();
    void handleFrame(const QByteArray &frame);
    void writeToBuffer(const QByteArray &frame, bool control = false);
    void timerEvent(QTimerEvent *e);
    QList<QByteArray > dataList;
    QByteArray data;
    static SerialContext *obj;
    QSerialPort *serial;
    friend class MainPage;
    static inline char boolToDeviceCmd(bool on)
    {
        // 0x01 = 开，0x02 = 关（硬件手册里约定的值）
        return static_cast<char>(on ? 0x01 : 0x02);
    }
};

#endif // SERIALCONTEXT_H