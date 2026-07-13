/***
 * \file    devices_fan.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   Mini·çÉÈÄ£¿éÇý¶¯
*/
#include "devices_fan.h"

void devices_fan_init()
{
	RCC->APB2ENR |= 1<<2;
	GPIOA->CRL &= 0XF0FFFFFF;
	GPIOA->CRL |= 0X03000000;
}

