/***
 * \file    devices_relay.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   ¼ÌµçÆ÷Ä£¿éÇý¶¯
*/
#include "devices_relay.h"

void devices_relay_init()
{
	RCC->APB2ENR |= 1<<2;
	GPIOA->CRL &= 0XFF00FFFF;
	GPIOA->CRL |= 0X00330000;
}
