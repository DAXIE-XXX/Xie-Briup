/***
 * \file    devices_usart02_debug.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-24
 * 
 * \brief   µ÷ÊÔ´®¿ÚÄ£¿é
 * 
*/
#ifndef DEVICES_USART03_DEVICE_H
#define DEVICES_USART03_DEVICE_H
#include "system_option.h"
#include <string.h>

#define DEVICE_BUFFER_LEN             256

extern u8 device_rx_flag;

void devices_usart03_device_init( u8 sclk, u32 baud);

void devices_usart03_device_send_ch(u8 ch);
void devices_usart03_device_send_str(char * str);
void devices_usart03_device_send_buf(u16 len, u8 * buf);
void devices_usart03_device_send_number(double num);

u8 devices_usart03_device_read_ch(void);
u16 devices_usart03_device_read_buf(u16 size, u8 * buf);

#endif

