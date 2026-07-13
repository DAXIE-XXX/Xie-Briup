/***
 * \file    devices_veml6070.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   紫外线传感器模块驱动
 *          器件型号：VEMl6070
 *          通信方式：IIC
 *          文件功能：获取紫外线强度数据
 *          器件地址：0X70
 *          附：紫外线强度对照表（参数选择为Rset=270KΩ(硬件焊接) IT=4t）
 *          数值											等级
 *          0~2240				 						 低
 *          2241~4482									 中等
 *          4483~5976									 高
 *          5977~8216                  非常高
 *          >8217											 火星上，别看了，保命要紧
*/
#ifndef DEVICES_VEML6070_H
#define DEVICES_VEML6070_H
#include "system_option.h"

/*******************************
函数名：devices_vm6070_set
函数功能：获取温湿度数据
函数返回值：成功返回0
*******************************/
unsigned char devices_vm6070_set(void);

/*******************************
函数名：devices_vm6070_measure
函数功能：获取温湿度数据
函数返回值：成功返回0
*******************************/
unsigned char devices_vm6070_measure(u16* Result);

#endif
