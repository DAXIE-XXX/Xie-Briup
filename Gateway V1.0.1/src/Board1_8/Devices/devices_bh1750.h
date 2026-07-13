/***
 * \file    devices_bh1750.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   软件模拟IIC驱动
*/
#ifndef DEVICES_BH1750_H
#define DEVICES_BH1750_H

#include "system_option.h"
/*******************************
函数名：devices_bh1750_init
函数功能：初始化BH1750
函数返回值：成功返回0
*******************************/
void devices_bh1750_init(void);

/*******************************
函数名：devices_bh1750_measure
函数功能：获取一次BH1750的测量值
函数返回值：成功返回0
注意：每次读取光照强度数据至少要间隔120ms
			在高精度模式下传感器转换一次数据需要至少120ms时间
*******************************/
unsigned char devices_bh1750_measure(float* Result);
/*******************************
函数名：devices_bh1750_measure
函数功能：获取一次BH1750的原始测量值
函数返回值：成功返回0
注意：每次读取光照强度数据至少要间隔120ms
			在高精度模式下传感器转换一次数据需要至少120ms时间
*******************************/
unsigned char devices_bh1750_original_measure(u16* Result);




#endif
