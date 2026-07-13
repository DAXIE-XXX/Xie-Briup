#include "serialcontext.h"
#include <QSerialPort>
#include <QSerialPortInfo>
SerialContext *SerialContext::obj = nullptr;
SerialContext::SerialContext(QObject *parent)
    : QObject{parent}
    ,serial{new QSerialPort}
{
    serial->setDataBits(QSerialPort::Data8);// 设置串口的数据位为 8 位数据位
    serial->setStopBits(QSerialPort::OneStop);// 设置串口的停止位为 1 位停止位
    serial->setParity(QSerialPort::NoParity);// 设置串口的校验位为无校验（None）
    serial->setFlowControl(QSerialPort::NoFlowControl);// 设置串口的流控（流量控制）为无流控
    connect(serial, &QSerialPort::readyRead, this, &SerialContext::readyReadSlot);
    startTimer(1000);
}

SerialContext *SerialContext::getObject()
{
    if(obj == nullptr)
        obj = new SerialContext;
    return obj;
}

QStringList SerialContext::getPortNames()
{
    QStringList list; // 声明一个空的字符串列表，用于存储将要找到的串口名称
    foreach (const QSerialPortInfo &info,
             QSerialPortInfo::availablePorts()) {
        list.append(info.portName());
    }
    return list;
}

bool SerialContext::openSerial(QString name, quint16 baudrate)
{
    if(serial->isOpen())
        serial->close();
    serial->setPortName(name);
    serial->setBaudRate(baudrate);
    return serial->open(QIODevice::ReadWrite);
}

void SerialContext::setLED2(int light)
{
    QByteArray cmd = "FE FE 00 FF FF 0A 10 12 00 FF";
    QByteArray data = QByteArray::fromHex(cmd);
    data[8] = (char)light;//255（FF）开 0关
    //serial->write(data);
    writeToBuffer(data, true);
}

void SerialContext::setRGB(int r, int g, int b)
{
    QByteArray cmd = "FE FE 00 FF FF 0C 10 15 00 00 00 FF";
    QByteArray data = QByteArray::fromHex(cmd);
    data[8] = (char)g;
    data[9] = (char)r;
    data[10] = (char)b;
    //serial->write(data);
    writeToBuffer(data, true);
}

void SerialContext::setFun(bool sw)
{
    QByteArray cmd = "FE FE 00 FF FF 0A 70 27 02 FF";
    QByteArray data = QByteArray::fromHex(cmd);
    data[8] = sw ? 0x01 : 0x02;
    //serial->write(data);
    writeToBuffer(data, true);
}

void SerialContext::setAlarmLight(bool sw)
{
    QByteArray cmd = "FE FE 00 FF FF 0A 50 22 02 FF";
    QByteArray data = QByteArray::fromHex(cmd);
    data[8] = sw ? 0x01 : 0x02;
    //serial->write(data);
    writeToBuffer(data, true);
}

void SerialContext::setAlarm(bool sw)
{
    QByteArray cmd = "FE FE 00 FF FF 0A 50 21 02 FF";
    QByteArray data = QByteArray::fromHex(cmd);
    data[8] = sw ? 0x01 : 0x02;
    //serial->write(data);
    writeToBuffer(data, true);
}

// ========== 新增传感器请求函数 ==========

void SerialContext::getPM25()  // PM2.5 - 设备ID 0x1E, 节点ID 0x40
{
    QByteArray cmd = "FE FE 00 FF FF 09 40 1E FF";
    QByteArray data = QByteArray::fromHex(cmd);
    //serial->write(data);
    writeToBuffer(data, false);
}

void SerialContext::getSmoke()  // 烟雾 - 设备ID 0x1C, 节点ID 0x30
{
    QByteArray cmd = "FE FE 00 FF FF 09 30 1C FF";
    QByteArray data = QByteArray::fromHex(cmd);
    //serial->write(data);
    writeToBuffer(data, false);
}

void SerialContext::getMethane()  // 甲烷 - 设备ID 0x1A, 节点ID 0x30
{
    QByteArray cmd = "FE FE 00 FF FF 09 30 1A FF";
    QByteArray data = QByteArray::fromHex(cmd);
    //serial->write(data);
    writeToBuffer(data, false);
}

void SerialContext::getLight()  // 光照 - 设备ID 0x2C, 节点ID 0x20
{
    QByteArray cmd = "FE FE 00 FF FF 09 20 2C FF";
    QByteArray data = QByteArray::fromHex(cmd);
    //serial->write(data);
    writeToBuffer(data, false);
}

void SerialContext::getCo2()  // CO2 - 设备ID 0x16, 节点ID 0x20
{
    QByteArray cmd = "FE FE 00 FF FF 09 20 16 FF";
    QByteArray data = QByteArray::fromHex(cmd);
    //serial->write(data);
    writeToBuffer(data, false);
}

void SerialContext::getFire()  // 火光 - 设备ID 0x1B, 节点ID 0x30
{
    QByteArray cmd = "FE FE 00 FF FF 09 30 1B FF";
    QByteArray data = QByteArray::fromHex(cmd);
    //serial->write(data);
    writeToBuffer(data, false);
}

void SerialContext::getTemperature()  // 温度1 - 设备ID 0x18, 节点ID 0x20
{
    QByteArray cmd = "FE FE 00 FF FF 09 20 18 FF";
    QByteArray data = QByteArray::fromHex(cmd);
    //serial->write(data);
    writeToBuffer(data, false);
}

void SerialContext::getHumidity()  // 湿度1 - 设备ID 0x19, 节点ID 0x20
{
    QByteArray cmd = "FE FE 00 FF FF 09 20 19 FF";
    QByteArray data = QByteArray::fromHex(cmd);
    //serial->write(data);
    writeToBuffer(data, false);
}

// ========== 数据接收处理 ==========

void SerialContext::readyReadSlot()
{
    data.append(serial->readAll());
    handleData();
}

void SerialContext::handleData()
{
    for(int i = 1; i < data.length(); i++)
    {
        if((unsigned char)data.at(i) == 0xEF &&
            (unsigned char)data.at(i-1) == 0xEF)
        {
            //帧头
            if(i != 1)
            {
                data = data.mid(i-1, data.length());
                i = 0;
                continue;
            }
            if(data.length() < 9)
                return;
            int len = (unsigned char)data.at(5);
            if(data.length() < len)
                return;
            if((unsigned char)data.at(len-1) == 0xFF)
            {
                //帧尾
                QByteArray frame = data.mid(0, len);
                handleFrame(frame);
                data = data.mid(len, data.length());
                continue;
            }
            else
            {
                data = data.mid(1, data.length());
                i = 0;
                continue;
            }
        }
    }
}

void SerialContext::handleFrame(const QByteArray &frame)
{
    quint8 devicedId = (unsigned char)frame.at(7);

    switch(devicedId)
    {
    case 0x16:  // CO2 - 两字节整数，低字节在前
    {
        int value = (quint8)frame.at(8) + (quint8)frame.at(9) * 256;
        emit getCo2Sig(value);
        break;
    }
    case 0x18:  // 温度1 - 整数+小数（放大100倍）
    {
        double value = (quint8)frame.at(8) + (double)(quint8)frame.at(9) / 100.0;
        emit getTemperatureSig(value);
        break;
    }
    case 0x19:  // 湿度1 - 整数+小数（放大100倍）
    {
        double value = (quint8)frame.at(8) + (double)(quint8)frame.at(9) / 100.0;
        emit getHumiditySig(value);
        break;
    }
    case 0x1A:  // 甲烷 - 0x01正常 0x02异常
    {
        bool value = (quint8)frame.at(8) == 0x02;  // true表示异常
        emit getMethaneSig(value);
        break;
    }
    case 0x1B:  // 火光 - 0x01正常 0x02异常
    {
        bool value = (quint8)frame.at(8) == 0x02;  // true表示检测到火光
        emit getFireSig(value);
        break;
    }
    case 0x1C:  // 烟雾 - 0x01正常 0x02异常
    {
        bool value = (quint8)frame.at(8) == 0x02;  // true表示检测到烟雾
        emit getSmokeSig(value);
        break;
    }
    case 0x1E:  // PM2.5 - 两字节整数，低字节在前
    {
        int value = (quint8)frame.at(8) + (quint8)frame.at(9) * 256;
        emit getPM25Sig(value);
        break;
    }
    case 0x2C:  // 光照 - 两字节整数，低字节在前
    {
        int value = (quint8)frame.at(8) + (quint8)frame.at(9) * 256;
        emit getLightSig(value);
        break;
    }
    default:
        break;
    }
}

void SerialContext::writeToBuffer(const QByteArray &frame, bool control)
{
    if(control)
        dataList.insert(0, frame);
    else
        dataList.append(frame);
}

void SerialContext::timerEvent(QTimerEvent *e)
{
    if(dataList.isEmpty() || !serial->isOpen())
        return;
    QByteArray frame = dataList.first();
    dataList.removeFirst();
    serial->write(frame);
}
















