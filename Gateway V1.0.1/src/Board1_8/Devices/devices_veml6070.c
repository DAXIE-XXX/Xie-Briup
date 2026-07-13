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
#include "devices_software_iic.h"
#include "devices_veml6070.h"

#define VEML6070_ADDR	0X70	//器件地址
#define VEML6070_DH		0X73	//读温度
#define VEML6070_DL		0X71	//读湿度
/*******************************
函数名：devices_vm6070_set
函数功能：获取紫外线数据
函数返回值：成功返回0			 
*******************************/
unsigned char devices_vm6070_set(void)
{
    devices_software_iic_start();
    devices_software_iic_send_byte(0X70);
    if (devices_software_iic_wait_ack()) 
    {
        devices_software_iic_stop();
        return 1;
    }
    devices_software_iic_send_byte(0X0E);//此处模式配置，关闭ACK功能 IT配置为4T
    if (devices_software_iic_wait_ack()) 
    {
        devices_software_iic_stop();
        return 1;
    }
    devices_software_iic_stop();
    return  0;
}
/*******************************
函数名：devices_vm6070_measure
函数功能：获取紫外线强度
函数返回值：成功返回0
*******************************/
unsigned char devices_vm6070_measure(u16* Result)
{
    u8 tempH,tempL;
    /*********获取高8位数据*********/
    devices_software_iic_start();
    devices_software_iic_send_byte(0X73);
    if (devices_software_iic_wait_ack()) 
    {
        devices_software_iic_stop();
        return 1;
    }
    tempH=devices_software_iic_read_byte(1);//读取总线数据
    devices_software_iic_stop();
    /*********获取低8位数据*********/
    devices_software_iic_start();
    devices_software_iic_send_byte(0X71);
    if (devices_software_iic_wait_ack()) 
    {
        devices_software_iic_stop();
        return 1;
    }
    tempL=devices_software_iic_read_byte(1);//读取总线数据
    devices_software_iic_stop();
    *Result = (tempH<<8)|tempL;
    return 0;
}
