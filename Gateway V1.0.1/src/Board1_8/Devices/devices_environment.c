/***
 * \file    devices_environment.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   环境监测模块驱动
*/
#include "devices_environment.h"

void devices_environment_init()
{
	RCC->APB2ENR |= 1<<3;//使能GPIOB时钟
	GPIOB->CRL &= 0X00FFFF00;
	GPIOB->CRL |= 0X88000088;
	GPIOB->ODR |= (1<<7)|(1<<6)|(1<<1)|(1<<0);//使能上拉输入
}
