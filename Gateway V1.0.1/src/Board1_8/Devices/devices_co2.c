/***
 * \file    devices_co2.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   二氧化碳传感器驱动
 *          器件型号：---------
 *          通信方式：串口
 *          文件功能：获取二氧化碳浓度数据
 *          获取二氧化碳数据（被动式）数据帧：
 *          	0X42 0X4D 0XE3 0X00 0X00 0X01 0X72
*/
#include "devices_co2.h"

unsigned char CO2_R_COM[7]={0X42,0X4D,0XE3,0X00,0X00,0X01,0X72};

u16 devices_co2_measure(void)
{
	u16 temp = 0;
    u8 buf[100], data_l, data_h;
    u8 count = 0, i, len = 0;
	devices_usart03_device_send_buf(7,CO2_R_COM);//发送获取数据指令
    while(!len){
        len = devices_usart03_device_read_buf(100, buf);
        count ++;
        system_delay_ms(2);
        if( count > 250)
            return 0;
    }
    for( i = 0;i < len;i ++)
    {
        if(buf[i] == 0x42 && buf[i+1] == 0x4D)
        {
            data_h = buf[i+4];
            data_l = buf[i+5];
            break;
        }
    }
    if(i == len)
        return 0;
    temp = data_h<<8 | data_l;
    return temp;
}

void devices_co2_init()
{
	devices_usart03_device_init(36,9600);
}


