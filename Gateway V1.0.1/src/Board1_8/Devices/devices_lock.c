/***
 * \file    devices_lock.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   Mini风扇模块驱动
*/
#include "devices_lock.h"

void devices_lock_init()
{
	RCC->APB2ENR |= 1<<3;//使能GPIOB时钟
	GPIOB->CRH &= 0XFFFFFF0F;
	GPIOB->CRH |= 0X00000030;
	GPIOB->ODR &=~(1<9);
}
