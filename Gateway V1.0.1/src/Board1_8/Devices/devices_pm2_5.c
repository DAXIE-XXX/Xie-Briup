/***
 * \file    devices_pm2_5.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   pm2.5传感器模块驱动
*/
#include "devices_pm2_5.h"
#include "devices_usart03_device.h"

//初始化PM2.5传感器
void devices_pm2_5_init()
{
	devices_usart03_device_init(36, 9600);
}
//获取传感器数据值
u16 devices_pm2_5_measure(void)
{
    u8 len = 0, buf[100], data_h, data_l;
    u16 temp;
    u8 count = 0, i;
    while(!len){
        len = devices_usart03_device_read_buf(100, buf);
        count ++;
        if( count > 250)
            return 0;
    }
    for( i = 0;i < len;i ++)
    {
        if(buf[i] == 0xaa && buf[i+1] == 0xc0)
        {
            data_h = buf[i+3];
            data_l = buf[i+2];
            break;
        }
    }
    if(i == len)
        return 0;
    temp = data_h<<8 | data_l;
    return temp;
}


