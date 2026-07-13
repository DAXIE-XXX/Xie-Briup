/***
 * \file    devices_rgb_led.h
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
#ifndef DEVICES_RGB_LED_H
#define DEVICES_RGB_LED_H

#include "system_option.h"

#define R_LED			PAxOut(8)                   //红色LED  00
#define G_LED			PAxOut(11)									//绿色LED  00
#define B_LED			PAxOut(12)									//蓝色LED  00
#define LED_COLOR(clr)  do{R_LED = (clr&(1<<2))==0?1:0;G_LED = (clr&(1<<1))==0?1:0;B_LED = (clr&(1<<0))==0?1:0;}while(0)

void devices_rgb_led_init(void);

#endif
