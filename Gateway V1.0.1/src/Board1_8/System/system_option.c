/***
 * \file    system_option.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-27
 * 
 * \brief   位带操作、单片机系统配置
 * 
*/
#include "system_option.h"
static volatile u8 NVIC_Group;	//保存分组号
void system_option_nvic_priority_group_config(u8 group)    //设置NVIC分组
{ 
	//SCB->AIRCR
	//System Control Block	系统控制块(区)
	u32 temp;
	NVIC_Group = group;
	//通过STM32组号获取CM3内核中断分组号
	group = ~group & 0x07;
	
	temp = SCB->AIRCR;
	temp &= 0x0000f8ff;
	temp |= group << 8;
	temp |= 0x05fa0000;
	SCB->AIRCR = temp;
}

void system_option_nvic_priority_set(u8 preemptionPriority,u8 subPriority,u8 channel)  //设置中断
{ 
	//NVIC->IP寄存器组
	//根据中断分组指定4位有效位的抢占优先级位数和...
	u8 tmp;
	//指定有效位数的临时变量
	u8 tmpptr, tmpsub;
	tmpptr = 0x04 - NVIC_Group;
	tmpsub = 0x0f >> NVIC_Group;
	
	tmp = preemptionPriority << tmpptr;
	tmp |= subPriority & tmpsub;
	tmp <<= 4;
	NVIC->IP[channel] = tmp;
	NVIC->ISER[channel>>0x05] |= 1 << (channel & 0x1f);  				   
}

void system_option_jtag_set(u8 mode)
{
    u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //开启辅助时钟
	AFIO->MAPR&=0XF8FFFFFF; //清除MAPR的[26:24]
	AFIO->MAPR|=temp;       //设置jtag模式
}
