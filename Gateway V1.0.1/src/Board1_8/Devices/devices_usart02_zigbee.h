/***
 * \file    devices_usart02_zigbee.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-23
 * 
 * \brief   Zigbee´®¿ÚÇý¶¯Ä£¿é
 * 
*/
#ifndef DEVICES_USART02_ZIGBEE_H
#define DEVICES_USART02_ZIGBEE_H
#include "system_option.h"
#include "system_delay.h"
#include <string.h>

#define ZIGBEE_BUFFER_LEN             256
#define ZIGBEE_RESET()  do{PAout(1) = 0;system_delay_ms(100);PAout(1) = 1;}while(0);

void devices_usart02_zigbee_init( u8 sclk, u32 baud);

void devices_usart02_zigbee_send_ch(u8 ch);
void devices_usart02_zigbee_send_str(char * str);
void devices_usart02_zigbee_send_buf(u16 len, u8 * buf);
void devices_usart02_zigbee_send_number(double num);

u8 devices_usart02_zigbee_read_ch(void);
u16 devices_usart02_zigbee_read_buf(u16 size, u8 * buf);

#endif
