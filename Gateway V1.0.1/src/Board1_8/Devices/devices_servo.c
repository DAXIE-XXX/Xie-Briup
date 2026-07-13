/***
 * \file    devices_servo.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   舵机模块驱动
*/
#include "devices_servo.h"

void devices_servo_init(u16 arr, u16 psc)
{
	RCC->APB2ENR |= 1<<3;
	RCC->APB2ENR |= 1<<0;//使能AFIO时钟
	GPIOB->CRH &= 0XFFFFF0FF;
	GPIOB->CRH |= 0X00000B00;//复用功能推挽输出
	AFIO->MAPR &=~(0X03<<8);
	AFIO->MAPR |= 2<<8;//将TIM2_CH3重映射至PB10
	
	/**********以下是定时器的初始化**************/
	RCC->APB1ENR|=1<<0;			//TIM2时钟使能
	TIM2->ARR=arr;					//设定计数器自动重装值
	TIM2->PSC=psc;					//预分频器不分频
	
	TIM2->CCMR2|=7<<4;			//CH3 PWM2模式
	TIM2->CCMR2|=1<<3;			//CH3预装载使能
	TIM2->CCER|=1<<8;				//OC3输出使能
	TIM2->CR1=0x0080;				//ARPE使能
	TIM2->CR1|=0X01;				//使能定时器2
	TIM2->CCR3=18800;				//默认上电关闭出库闸机
}
//闸机打开
void devices_servo_on(void)
{
	TIM2->CCR3=19450;//打开出库闸机
}
//闸机关闭
void devices_servo_off(void)
{
	TIM2->CCR3=18800;//关闭出库闸机
}

