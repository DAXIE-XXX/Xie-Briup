/***
 * \file    devices_ws2812b.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   炫彩LED灯模块驱动
*/
#ifndef DEVICES_WS2812B_H
#define DEVICES_WS2812B_H

#include "system_option.h"

#define X_LEN			8	//矩阵LEDX长度
#define Y_LEN			8 //矩阵LEDY长度
																							
void devices_ws2812b_init(void);
void devices_ws2812b_en(void);
/*********************************************
函数名     devices_ws2812b_pixel_en
函数功能：根据起始坐标点点亮LED
输入参数: x,y为起始坐标点
					len：以起始坐标点开始（包括）要点亮LED的个数。
					RGB_Value:将要显示的颜色 bit0-7蓝色 bit8-15绿色 bit16-23红色  ff显示对应值
					mode：显示模式 
*********************************************/
void devices_ws2812b_pixel_en(u8 x,u8 y,u8 len,u32* RGB_Value,u8 mode);
/*********************************************
函数名     devices_ws2812b_clear
函数功能： 熄灭所有的LED
*********************************************/
void devices_ws2812b_clear(void);
#endif
