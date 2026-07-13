/***
 * \file    devices_sht20.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   温湿度传感器模块驱动
 *          器件型号：SHT20
 *          通信方式：IIC
 *          文件功能：获取温湿度数据
 *          器件地址：0X80
 *          读取相对温度操作指令：0XF3(非保持主机)
 *          读取相对湿度操作指令：0XF5(非保持主机)
*******************************************************/
#include "devices_sht20.h"

#define SHT20_ADDR	0X80	//器件地址
#define SHT20_RT		0XF3	//读温度
#define SHT20_RH		0XF5	//读湿度
/*******************************
函数名：devices_sht20_measure
函数功能：获取温湿度数据
函数返回值：成功返回0
*******************************/
unsigned char devices_sht20_measure(float* Temperature, float* Humidity)
{
	unsigned char temp[2]={0};
	u16 ST=0;
	if(!devices_software_iic_single_read_Len(SHT20_ADDR,SHT20_RT,2,temp))
	{
		ST = (temp[0]<<8)|temp[1];
		ST&=~0X0003;
		*Temperature = ((float)ST * 0.00268127)-46.85;
	}
	else
	{
		*Temperature = 0;
		return 1;
	}
/******************获取湿度***************/
	if(!devices_software_iic_single_read_Len(SHT20_ADDR,SHT20_RH,2,temp))
	{
		ST = (temp[0]<<8)|temp[1];
		ST&=~0X0003;
		*Humidity = ((float)ST * 0.00190735)-6;
	}
	else
	{
		*Humidity = 0;
		return 1;
	}
	return 0;
}
/*******************************
函数名：devices_sht20_original_temp
函数功能：获取原始温度数据
函数返回值：成功返回0
*******************************/
unsigned char devices_sht20_original_temp(u16* Temperature)
{
	unsigned char temp[2]={0};
//	u16 ST=0;
	if(!devices_software_iic_single_read_Len(SHT20_ADDR,SHT20_RT,2,temp))
	{
		*Temperature = (temp[0]<<8)|temp[1];
		*Temperature&=~0X0003;
	}
	else
	{
		*Temperature = 0;
		return 1;
	}
	return 0;
}
/*******************************
函数名：devices_sht20_original_humi
函数功能：获取原始湿度数据
函数返回值：成功返回0
*******************************/
unsigned char devices_sht20_original_humi(u16* Humidity)
{
	unsigned char temp[2]={0};
//	u16 ST=0;
/******************获取湿度***************/
	if(!devices_software_iic_single_read_Len(SHT20_ADDR,SHT20_RH,2,temp))
	{
		*Humidity = (temp[0]<<8)|temp[1];
		*Humidity&=~0X0003;
	}
	else
	{
		*Humidity = 0;
		return 1;
	}
	return 0;
}

