/***
 * \file    devices_beep.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   蜂鸣器模块驱动
 *          器件型号：5V有源蜂鸣器
 *          通信方式：IO控制 与单片机的PA15引脚相连接
*/
#include "devices_beep.h"

void devices_beep_init()
{
	RCC->APB2ENR |= 1<<2;//使能GPIOA时钟
	GPIOA->CRH &= 0X0FFFFFFF;
	GPIOA->CRH |= 0X30000000;
	BEEP_CONTROL=0;
}

