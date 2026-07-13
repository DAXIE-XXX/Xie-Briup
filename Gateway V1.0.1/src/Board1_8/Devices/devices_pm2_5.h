/***
 * \file    devices_pm2_5.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   pm2.5传感器模块驱动
*/
#ifndef DEVICES_PM2_5_H
#define DEVICES_PM2_5_H

#include "system_option.h"


//初始化PM2.5传感器
void devices_pm2_5_init(void);

//获取传感器数据值
u16 devices_pm2_5_measure(void);




#endif
