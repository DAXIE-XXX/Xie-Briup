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
#ifndef DEVICES_USART01_DEBUG_H
#define DEVICES_USART01_DEBUG_H
#include "system_option.h"
#include "system_delay.h"
#include <string.h>

#define DEBUG_BUFFER_LEN             256

void devices_usart01_debug_init( u8 sclk, u32 baud);

void devices_usart01_debug_send_ch(u8 ch);
void devices_usart01_debug_send_str(char * str);
void devices_usart01_debug_send_buf(u16 len, u8 * buf);
void devices_usart01_debug_send_number(double num);

u8 devices_usart01_debug_read_ch(void);
u16 devices_usart01_debug_read_buf(u16 size, u8 * buf);

#endif

