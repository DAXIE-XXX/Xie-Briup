/***
 * \file    devices_pt4115.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-27
 * 
 * \brief   高亮LED灯驱动模块头文件
 * 
*/
#ifndef DEVICES_PT4115_H
#define DEVICES_PT4115_H

#include <stm32f10x.h>

void devices_pt4115_init(u32 arr, u32 psc);
/******************************************************
函数名：PT4115_Change
函数功能：通过改变PWM的占空比实现调节光线亮度
函数参数：
				L1：高亮LED光路1调节亮度值
				L2：高亮LED光路2调节亮度值
				L3：高亮LED光路3调节亮度值
				L4：高亮LED光路4调节亮度值
******************************************************/
void devices_pt4115_change(u8 L1,u8 L2,u8 L3,u8 L4);

#endif
