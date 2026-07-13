/***
 * \file    devices_sht20.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   温湿度传感器模块驱动
*/
#ifndef DEVICES_SHT20_H
#define DEVICES_SHT20_H
#include "devices_software_iic.h"

/*******************************
函数名：devices_sht20_measure
函数功能：获取温湿度数据
函数返回值：成功返回0
*******************************/
unsigned char devices_sht20_measure(float* Temperature, float* Humidity);
/*******************************
函数名：devices_sht20_original_temp
函数功能：获取原始温度数据
函数返回值：成功返回0
*******************************/
unsigned char devices_sht20_original_temp(u16* Temperature);
/*******************************
函数名：devices_sht20_original_humi
函数功能：获取原始湿度数据
函数返回值：成功返回0
*******************************/
unsigned char devices_sht20_original_humi(u16* Humidity);

#endif
