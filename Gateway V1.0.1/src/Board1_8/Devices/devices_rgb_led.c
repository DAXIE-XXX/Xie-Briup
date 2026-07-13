/***
 * \file    devices_rgb_led.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   三色LED模块驱动
*******************************************************/
#include "devices_rgb_led.h"

void devices_rgb_led_init()
{
	RCC->APB2ENR|=1<<2;//使能PORTA时钟
	GPIOA->CRH &= 0XFFF00FF0;
	GPIOA->CRH |= 0X00033003;//PA8 11 12推挽输出
	R_LED = 1;
	G_LED = 1;
	B_LED = 1;
}

