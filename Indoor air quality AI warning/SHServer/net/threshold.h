#ifndef THRESHOLD_H
#define THRESHOLD_H

#include <QString>

struct Threshold {
    int co2Max = 1000;        // ppm
    int pm25Max = 50;         // µg/m³
    double tempMax = 30.0;    // ℃
    double tempMin = 18.0;    // ℃
    bool methaneEnable = true; // 甲烷是否启用检测
    int cooldownSeconds = 30;  // 告警冷却（秒），避免同一传感器频繁触发
    int fanCooldownSeconds = 30; // 风扇关闭延迟（秒），数值恢复正常后再运行N秒后关闭
};

#endif // THRESHOLD_H