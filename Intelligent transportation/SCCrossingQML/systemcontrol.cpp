#include "systemcontrol.h"
#include "datacontext.h"
#include "netcontext.h"
#include "configcontext.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

SystemControl *SystemControl::obj = nullptr;

SystemControl::SystemControl(QObject *parent)
    : QObject{parent}
{
    ConfigContext::getGreenTime(greenTime);
    // 计算缓冲区大小：每个方向 (绿灯时间+黄灯时间) * 2 (因为500ms一次) * 2 (横向+纵向)
    bufSize = (greenTime + 3) * 2 * 2;

    // 分配所有缓冲区内存
    autoModeHorBuf = new int[bufSize];
    autoModeVerBuf = new int[bufSize];
    horModeHorBuf = new int[bufSize];
    horModeVerBuf = new int[bufSize];
    verModeHorBuf = new int[bufSize];
    verModeVerBuf = new int[bufSize];
    noModeHorBuf = new int[bufSize];
    noModeVerBuf = new int[bufSize];
    nightModeHorBuf = new int[bufSize];
    nightModeVerBuf = new int[bufSize];

    // ==================== 初始化自动模式缓冲区 ====================
    int index = 0;

    // 横向：绿灯常亮 (0=绿灯)
    for(int i = 0; i < (greenTime - 3) * 2; i++){
        autoModeHorBuf[index++] = 0;
    }
    // 横向：绿灯闪烁 (0=绿灯, -1=灭灯)
    int flag = 0;
    for(int i = 0; i < 6; i++){
        if(flag == 0){
            autoModeHorBuf[index++] = -1;  // 灭灯
            flag = 1;
        }
        else{
            autoModeHorBuf[index++] = 0;   // 绿灯
            flag = 0;
        }
    }
    // 横向：黄灯闪烁 (1=黄灯, -1=灭灯)
    flag = 0;
    for(int i = 0; i < 6; i++){
        if(flag == 0){
            autoModeHorBuf[index++] = -1;  // 灭灯
            flag = 1;
        }
        else{
            autoModeHorBuf[index++] = 1;   // 黄灯
            flag = 0;
        }
    }
    // 横向：红灯常亮 (2=红灯)
    for(int i = 0; i < (greenTime + 3) * 2; i++){
        autoModeHorBuf[index++] = 2;
    }

    // 纵向：红灯常亮（前8秒）(2=红灯)
    index = 0;
    for(int i = 0; i < (greenTime + 3) * 2; i++){
        autoModeVerBuf[index++] = 2;
    }
    // 纵向：绿灯常亮 (0=绿灯)
    for(int i = 0; i < (greenTime - 3) * 2; i++){
        autoModeVerBuf[index++] = 0;
    }
    // 纵向：绿灯闪烁 (0=绿灯, -1=灭灯)
    flag = 0;
    for(int i = 0; i < 6; i++){
        if(flag == 0){
            autoModeVerBuf[index++] = -1;  // 灭灯
            flag = 1;
        }
        else{
            autoModeVerBuf[index++] = 0;   // 绿灯
            flag = 0;
        }
    }
    // 纵向：黄灯闪烁 (1=黄灯, -1=灭灯)
    flag = 0;
    for(int i = 0; i < 6; i++){
        if(flag == 0){
            autoModeVerBuf[index++] = -1;  // 灭灯
            flag = 1;
        }
        else{
            autoModeVerBuf[index++] = 1;   // 黄灯
            flag = 0;
        }
    }

    // ==================== 初始化横向通行模式缓冲区 ====================
    index = 0;
    // 横向：绿灯常亮 (0=绿灯)
    for(int i = 0; i < bufSize / 2; i++){
        horModeHorBuf[index++] = 0;
    }
    // 横向：黄灯闪烁（最后3秒）(1=黄灯, -1=灭灯)
    flag = 0;
    for(int i = 0; i < 6; i++){
        if(flag == 0){
            horModeHorBuf[index++] = 1;   // 黄灯
            flag = 1;
        }
        else{
            horModeHorBuf[index++] = -1;  // 灭灯
            flag = 0;
        }
    }

    // 纵向：红灯常亮（全程）(2=红灯)
    index = 0;
    for(int i = 0; i < bufSize; i++){
        horModeVerBuf[index++] = 2;
    }

    // ==================== 初始化纵向通行模式缓冲区 ====================
    index = 0;
    // 横向：红灯常亮（全程）(2=红灯)
    for(int i = 0; i < bufSize; i++){
        verModeHorBuf[index++] = 2;
    }

    // 纵向：绿灯常亮 (0=绿灯)
    index = 0;
    for(int i = 0; i < bufSize / 2; i++){
        verModeVerBuf[index++] = 0;
    }
    // 纵向：黄灯闪烁（最后3秒）(1=黄灯, -1=灭灯)
    flag = 0;
    for(int i = 0; i < 6; i++){
        if(flag == 0){
            verModeVerBuf[index++] = 1;   // 黄灯
            flag = 1;
        }
        else{
            verModeVerBuf[index++] = -1;  // 灭灯
            flag = 0;
        }
    }

    // ==================== 初始化全灭模式缓冲区 ====================
    for(int i = 0; i < bufSize; i++){
        noModeHorBuf[i] = -1;   // 横向灭灯
        noModeVerBuf[i] = -1;   // 纵向灭灯
    }

    // ==================== 初始化夜间模式缓冲区 ====================
    // 夜间模式：黄灯闪烁（所有方向）(1=黄灯, -1=灭灯)
    flag = 0;
    for(int i = 0; i < bufSize; i++){
        if(flag == 0){
            nightModeHorBuf[i] = 1;   // 横向黄灯
            nightModeVerBuf[i] = 1;   // 纵向黄灯
            flag = 1;
        }
        else{
            nightModeHorBuf[i] = -1;  // 横向灭灯
            nightModeVerBuf[i] = -1;  // 纵向灭灯
            flag = 0;
        }
    }

    // 默认使用自动模式缓冲区
    currentHorBuf = autoModeHorBuf;
    currentVerBuf = autoModeVerBuf;
    currentIndex = 0;

    startTimer(500);
    connect(NetContext::getObject(), &NetContext::crossingModeSet, this, &SystemControl::crossingModeSetSlot);

    connect(DataContext::getObject(), &DataContext::lightColorChanged,this, &SystemControl::lightColorChangedslot);
}

SystemControl *SystemControl::getObject()
{
    if(obj == nullptr)
        obj = new SystemControl;
    return obj;
}

void SystemControl::system_init()
{
    DataContext::getObject();
    NetContext::getObject();
    ConfigContext::getCrossingId(croId);

    emit crossingIdChanged(croId);

    int id,roadId;
    bool show;
    ConfigContext::getLeftLightInfo(id, show, roadId);
    DataContext::getObject()->setLeftLightInfo(id, show, roadId);

    ConfigContext::getRightLightInfo(id, show, roadId);
    DataContext::getObject()->setRightLightInfo(id, show, roadId);

    ConfigContext::getUpLightInfo(id, show, roadId);
    DataContext::getObject()->setUpLightInfo(id, show, roadId);

    ConfigContext::getDownLightInfo(id, show, roadId);
    DataContext::getObject()->setDownLightInfo(id, show, roadId);
}

void SystemControl::systen_start()
{
    QString ip;
    quint16 port;
    ConfigContext::getServerInfo(ip, port);
    NetContext::getObject()->connectToServer(ip, port);
}

void SystemControl::setMode(LightMode newMode)
{
    if (mode == newMode) return;

    mode = newMode;
    currentIndex = 0;

    switch(mode){
    case AutoMode:
        currentHorBuf = autoModeHorBuf;
        currentVerBuf = autoModeVerBuf;
        break;
    case HorMode:
        currentHorBuf = horModeHorBuf;
        currentVerBuf = horModeVerBuf;
        break;
    case VerMode:
        currentHorBuf = verModeHorBuf;
        currentVerBuf = verModeVerBuf;
        break;
    case NoMode:
        currentHorBuf = noModeHorBuf;
        currentVerBuf = noModeVerBuf;
        break;
    case NightMode:
        currentHorBuf = nightModeHorBuf;
        currentVerBuf = nightModeVerBuf;
        break;
    }
    // 立即应用新模式的第一帧
    DataContext::getObject()->setLightColor(
        currentHorBuf[currentIndex], currentVerBuf[currentIndex]);
    // 向服务器发送模式状态
    NetContext::getObject()->sendModeStatus(croId, mode);
    qDebug() << "Mode changed to:" << mode << ", notification sent to server";

    emit modeChanged((int)mode);
}

void SystemControl::crossingModeSetSlot(int id, int mode)
{
    if(id != croId)
        return;
    setMode((LightMode)mode);
}

void SystemControl::lightColorChangedslot()
{
    QJsonArray arr = DataContext::getObject()->getLightList();
    for(int i = 0; i < arr.size(); i++){
        QJsonObject od = arr.at(i).toObject();
        if(od.value("show").toBool() == false)
            continue;
        int id = od.value("id").toInt();
        int roadId = od.value("roadId").toInt();
        int color = od.value("color").toInt();

        // 不再交换颜色，直接发送原始颜色值
        // 颜色定义: 0=绿灯, 1=黄灯, 2=红灯, -1=灭灯
        qDebug() << "Sending light color - id:" << id << "roadId:" << roadId << "color:" << color;
        NetContext::getObject()->sendLightColorChanged(id, roadId, color);
    }
}

void SystemControl::timerEvent(QTimerEvent *e)
{
    // 使用当前缓冲区设置灯光
    DataContext::getObject()->setLightColor(
        currentHorBuf[currentIndex], currentVerBuf[currentIndex]);

    currentIndex++;
    if(currentIndex >= bufSize)
        currentIndex = 0;
}