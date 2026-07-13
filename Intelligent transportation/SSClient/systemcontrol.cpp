#include "systemcontrol.h"
#include "netcontext.h"
#include "datacontext.h"

SystemControl *SystemControl::obj = nullptr;

SystemControl::SystemControl(QObject *parent)
    : QObject{parent}
{
    connect(NetContext::getObject(), &NetContext::lightColorChanged,
            this, &SystemControl::lightColorChangedSlot);
}

SystemControl *SystemControl::getObject()
{
    if(obj == nullptr)
        obj = new SystemControl;
    return obj;
}

void SystemControl::lightColorChangedSlot(int id, int roadId, int color)
{
    // 直接传递颜色值，不做任何转换
    // 颜色定义: 0=green, 1=yellow, 2=red, -1=off
    qDebug() << "SystemControl: lightColorChanged - id:" << id << "roadId:" << roadId << "color:" << color;
    DataContext::getObject()->setLightColor(id, color);
}