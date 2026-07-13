/***
 * \file    devices_step_motor.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   步进电机模块
 *          器件型号：A4988
 *          通信方式：PMW
 *          文件功能：控制步进电机旋转
 *          驱动模块引脚功能说明：
 *          EN								使能端，接地使能     			-->PB0
 *          STEP							驱动单片机脉冲信号输入			-->PB9 -->TIM4_CH4
 *          DIR								改变步进电机旋转方向				-->PB13
 *          SLEEP                                       -->PB8
 *          RESET                                       -->PB7
 *          MS1、MS2、MS3			细分配置，细分越高则转动更慢精度越高 MS1-->PB1,MS2-->PB5、MS3-->PB6
 *          配置模式：
 *          MS1 MS2 MS3       细分数
 *          0		0		0						1
 *          1		0		0						2
 *          0		1		0						4
 *          1		1		0						8
 *          1		1		1						16
*/
#include "devices_step_motor.h"

/******************************************
函数名：delays_step_motor_init
函数功能：初始化A4988步进电机驱动芯片
*******************************************/
void  delays_step_motor_init()
{
	RCC->APB2ENR|=1<<3;
    RCC->APB2ENR|=1<<2;//使能PB端口时钟
    RCC->APB1ENR|=(1<<1)|(1<<2);//使能定时器3,4的时钟
    
	GPIOB->CRL&=0X000FFF00;
	GPIOB->CRL|=0X33300033;
	GPIOB->CRH&=0XFF0FFFF0;
	GPIOB->CRH|=0X00300003;
    
    GPIOB->CRH&=0XFFFFFF0F;
    GPIOB->CRH|=0X000000B0;//复用功能输出     

    TIM3->PSC = 7199;//预分频值,每100us计数一次
    TIM3->ARR = 99; //重装值，决定单脉冲周期     

    TIM3->CR1|=0<<4;//向上计数模式     
    TIM3->CR1|=1<<3;//设置TIM3在单脉冲模式，且是单一的脉冲，在下一个更新事件停止          
    TIM3->CR2|=4<<4; //OC1REF signal is used as the trigger output (TRGO).单一模式不需要设置

    TIM3->CCMR1|=1<<3;//使能定时器3的通道1预装载寄存器
    TIM3->CCMR1|=7<<4;//在向上计数时，一旦TIMx_CNT>TIMx_CCR1时通道1为有效电平
    //TIM3->CCER |=1<<0;//OC1输出使能
    TIM3->CCER|=0<<1;//有效电平为高
    TIM3->CCR1 = 20;//比较捕获1的预装载值，决定单脉冲占空比，这个20就是低电平延续的来源 
    TIM3->DIER |= 1<<0;//允许更新中断
    TIM3->CR1|=1<<0;//使能定时器3

    //TIM4工作在从模式门控模式下
    TIM4->CR1|=0<<4;//向上计数模式
    TIM4->PSC = 719;//预分频值，10us计数一次
    TIM4->ARR = 29;//重装值，决定PWM周期

    TIM4->SMCR|=5;//TIM4为门控模式        
    TIM4->SMCR|=1<<7;//使能TIM4的主从模式        
    TIM4->SMCR|=2<<4;//内部触发，TIM3为主设备 TIM4为从设备

    TIM4->CCMR2|=1<<11;//使能定时器4的通道4预装载寄存器
    TIM4->CCMR2|=7<<12;//在向上计数时，一旦TIMx_CNT>TIMx_CCR1时通道1为有效电平
    TIM4->CCER|=1<<12;//OC1输出使能
    TIM4->CCER|=0<<13;//有效电平为高
    TIM4->CCR4 = 15;//比较捕获1的预装载值，占空比为50%
    TIM4->CR1|=1<<0;//使能TIM4
    system_option_nvic_priority_set(2,2,TIM3_IRQn);
    
	A4988_EN = 	1;//使能输出 1不使能 0使能
	A4988_MS1 = 0;
	A4988_MS2 = 0;
	A4988_MS3 = 0;//1细分
	A4988_DIR = 1;//方向设置
	A4988_SLEEP=1;
	A4988_RESET=1;
}

void TIM3_IRQHandler()
{
	if(TIM3->SR & 0X0001)
	{
		A4988_EN = 1;//除能步进电机驱动输出
	}
	TIM3->SR&=~(1<<0);//清除中断标志位
}
/*********************************************************
名称：delays_step_motor_control(u16 cycle, u16 pulse_num)
说明：生成指定个数脉冲，每个脉冲周期为cycle毫秒，脉冲个数生成的个数
      和单脉冲高电平时间有关系，脉冲个数就由高电平时间来确定
参数cycle：为TIM3一个脉冲周期,单位(us)
参数pulse_num：为脉冲个数，决定步进电机步数
返回值：无
*********************************************************/
void delays_step_motor_control(u16 cycle, u16 pulse_num, u8 dir)
{
    u16 arr3 = 0;
    u32 time = 0;
    A4988_EN = 	0;//使能步进电机驱动
    A4988_DIR = dir;
    if(pulse_num)
    { 
        time = cycle * pulse_num / 100;//预分频为720,100us计数一次
        arr3 = cycle / 10;             //预分频为72,10us计数一次 
        TIM3->ARR=time+1;
        TIM3->CCR1=1;
        TIM4->ARR=arr3;
        TIM4->CCR4=arr3/2;
        TIM4->CNT = 0;
        //TIM_SetAutoreload(TIM2, time + 19);//低电平周期加高电平周期
        //TIM_SetAutoreload(TIM3, arr3 - 1);
        //TIM_SetCompare1(TIM3,arr3 / 2); //设置PWM占空比为50%
        TIM3->CR1|=1<<0;//使能TIM3
    }
}

