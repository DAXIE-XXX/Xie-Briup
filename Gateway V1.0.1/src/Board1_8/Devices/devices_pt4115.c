/***
 * \file    devices_pt4115.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 *
 * \brief   高亮LED
 *          器件型号：PT4115
 *          通信方式：PWM
 *          文件功能：通过控制PT4115管脚上的电压实现LED亮度调节
 *          					电压越低亮度越低。
 *          管脚连接：
 *          				灯路1 -->TIM3_CH1 -->PA6
 *          				灯路2 -->TIM3_CH2 -->PA7
 *          				灯路3 -->TIM3_CH3 -->PB6
 *          				灯路4 -->TIM3_CH4 -->PB1
*******************************************************/
#include "devices_pt4115.h"

void devices_pt4115_init(u32 arr, u32 psc)
{
	RCC->APB1ENR|=1<<1;		//TIM3时钟使能
	RCC->APB1ENR|=1<<2;		//TIM4时钟使能
	RCC->APB2ENR|=1<<3;		//使能PORTB时钟
	RCC->APB2ENR|=1<<2;		//使能PORTA时钟
	//GPIOA->CRL&=0XF0FFFFFF;											
	//GPIOA->CRL|=0X0B000000;//PA6复用功能输出
	GPIOB->CRL&=0XF0FFFF00;											
	GPIOB->CRL|=0X0B0000BB;//PB0 1 6复用功能输出
	
	TIM3->ARR=arr;				//设定计数器自动重装值 
	TIM3->PSC=psc;				//预分频器分频系数 
	
	TIM4->ARR=arr;				//设定计数器自动重装值 
	TIM4->PSC=psc;				//预分频器分频系数 
	/*****************TIM3_CH1配置*****************/
	//TIM3->CCMR1|=6<<4;  	//CH1 PWM1模式		 
	//TIM3->CCMR1|=1<<3; 		//CH1 预装载使能	
	/*****************TIM3_CH3配置*****************/
	TIM3->CCMR2|=6<<4;  	//CH3 PWM1模式		 
	TIM3->CCMR2|=1<<3; 		//CH3 预装载使能
	/*****************TIM3_CH4配置*****************/
	TIM3->CCMR2|=6<<12;  	//CH4 PWM1模式		 
	TIM3->CCMR2|=1<<11; 	//CH4 预装载使能
	
	/*****************TIM4_CH1配置*****************/
	TIM4->CCMR1|=6<<4;  	//CH1 PWM1模式		 
	TIM4->CCMR1|=1<<3; 		//CH1 预装载使能
	
	//TIM3->CCER|=0<<1;   	//CH1 高电平有效	
	//TIM3->CCER|=1<<0;   	//CH1 输出使能
	TIM3->CCER|=0<<9;   	//CH3 高电平有效	
	TIM3->CCER|=1<<8;   	//CH3 输出使能
	TIM3->CCER|=0<<13;   	//CH4 高电平有效	
	TIM3->CCER|=1<<12;   	//CH4 输出使能
	TIM4->CCER|=0<<1;   	//CH1 高电平有效	
	TIM4->CCER|=1<<0;   	//CH1 输出使能
	TIM3->CR1=0X0080;   	//ARPE使能 
	TIM4->CR1=0X0080;			//ARPE使能
	TIM3->CR1|=1<<0;    	//使能定时器3
	TIM4->CR1|=1<<0;			//使能定时器4
    
	devices_pt4115_change( 0, 0, 0, 0);//使得所有路的高亮LED均熄灭
}
/******************************************************
函数名：devices_pt4115_change
函数功能：通过改变PWM的占空比实现调节光线亮度
函数参数：
				L1：高亮LED光路1调节亮度值
				L2：高亮LED光路2调节亮度值
				L3：高亮LED光路3调节亮度值
				L4：高亮LED光路4调节亮度值
******************************************************/
void devices_pt4115_change(u8 L1,u8 L2,u8 L3,u8 L4)
{
	TIM3->CCR1 = L1;
	TIM4->CCR1 = L2;
	TIM3->CCR3 = L3;
	TIM3->CCR4 = L4;
}
