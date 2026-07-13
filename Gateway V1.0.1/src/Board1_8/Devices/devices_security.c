/***
 * \file    devices_security.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   安防节点驱动
 *          文件功能：智能门窗相关传感器驱动配置
 *          管脚连接：
 *          				报警器		-->PB8
 *          				报警灯		-->PB9
 *          				光电开关	-->PB0
 *          				对射开关	-->PB1
*/
#include "devices_security.h"

void devices_security_init()
{
	RCC->APB2ENR |= 1<<3;//使能GPIOB时钟
	GPIOB->CRL &= 0XFFFFFF00;
	GPIOB->CRL |= 0X00000088;
	GPIOB->ODR |= (1<<1)|(1<<0);//使能上拉输入
	GPIOB->CRH &= 0XFFFFFF00;
	GPIOB->CRH |= 0X00000033;//配置为推挽输出
}
