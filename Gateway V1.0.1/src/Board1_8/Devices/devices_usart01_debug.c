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
 * \brief   调试串口模块
 * 
*/
#include "devices_usart01_debug.h"

u8 debug_rx_buf[DEBUG_BUFFER_LEN] = {0};	//接收缓冲区
u8 debug_rx_flag = 0;			//收到数据标志

void devices_usart01_debug_init( u8 sclk, u32 baud)
{
	double temp = (sclk * 1000000.0)/(16*baud);
	u16 mant = (u16) temp;
	u16 frac = (u16) ((temp - mant)*16);
	mant = mant << 4 | frac;
	
	RCC->APB2ENR |= 1<<14 | 1<<2;
	
	//PA9  -> U1.Tx  复用推挽输出50MHz
	//PA10 -> U1.Rx  上拉输入
	GPIOA->CRH &= 0xFFFFF00F;
	GPIOA->CRH |= 0x000008B0;
	GPIOA->ODR |= 1<<10;
	
	//开启DMA1的通道
	RCC->AHBENR |= 1<<0;
	
	USART1->BRR = mant;	//波特比率寄存器
	//使能串口  使能IDLE(总线空闲)中断  使能接收  使能发送
	USART1->CR1 = 1<<13 | 1<<4 | 1<<3 | 1<<2;
	//使能DMA接收   使能DMA发送
	USART1->CR3 = 1<<6 | 1<<7;
	
	DMA1_Channel4->CCR = 0;
	//Channel Memory Address Reg
	DMA1_Channel4->CMAR = 0;
	//Channel Peripheral Address Reg
	DMA1_Channel4->CPAR = (u32) &USART1->DR;
	DMA1_Channel4->CNDTR = 0;
	DMA1_Channel4->CCR |= 1<<12 | 1<<7 | 1<<4;
	
	DMA1_Channel5->CCR = 0;
	DMA1_Channel5->CMAR = (u32) debug_rx_buf;
	DMA1_Channel5->CPAR = (u32) &USART1->DR;
	//Channel Number Of Data Reg
	DMA1_Channel5->CNDTR = DEBUG_BUFFER_LEN;
	DMA1_Channel5->CCR |= 1<<13 | 1<<7 | 1<<0;
	//注册中断优先级
    system_option_nvic_priority_set(2,3,USART1_IRQn);
}

void devices_usart01_debug_send_ch(u8 ch)
{
	DMA1_Channel4->CCR &= ~0x01;
	DMA1_Channel4->CMAR = (u32) &ch;
	DMA1_Channel4->CNDTR = 1;
	DMA1_Channel4->CCR |= 0x01;
}

void devices_usart01_debug_send_str(char * str)
{
	u16 len = strlen( str);
	DMA1_Channel4->CCR &= ~0x01;
	DMA1_Channel4->CMAR = (u32) str;
	DMA1_Channel4->CNDTR = len;
	DMA1_Channel4->CCR |= 0x01;
	system_delay_ms(30);
}

void devices_usart01_debug_send_buf(u16 len, u8 * buf)
{
	//关闭U1.Tx对应的DMA1.CH4通道
	DMA1_Channel4->CCR &= ~0x01;
	//设置要发送的数据的内存首地址
	DMA1_Channel4->CMAR = (u32) buf;
	//设置要发送的数据数量
	DMA1_Channel4->CNDTR = len;
	//打开通道
	DMA1_Channel4->CCR |= 0x01;
	system_delay_ms(30);
}

void devices_usart01_debug_send_number(double num)
{
	//取浮点数后两位小数的值，转化为无符号整型
	u32 number = num * 100;
	u8 point = 0, i, len;
	u8 buf[30] = {0};
	if( num == 0)
	{
		devices_usart01_debug_send_ch('0');
		return;
	}
	//如果参数传进来不是整数，则设置一个标记量
	if( number % 100 != 0)
		point = 1;
	len = 0;
	while( number > 0)	//计算数据位数
	{
		len ++;
		number /= 10;
	}
	number = num * 100;
	if( point == 0)
	{
		len -= 2;
		number /= 100;
	}
	//将数值按位拆分，转化为字符形式，存储在buf中
	for( i = len; i > 0;i --)
	{
		buf[i-1] = number % 10 + '0';
		number /= 10;
	}
	
	//处理小数点
	number = num * 100;
	if( point != 0)
	{
		buf[len] = number % 10 + '0';
		buf[len-1] = number / 10 % 10 + '0';
		buf[len-2] = '.';
	}
	
	DMA1_Channel4->CCR &= ~0x01;
	DMA1_Channel4->CMAR = (u32) buf;
	DMA1_Channel4->CNDTR = len;
	DMA1_Channel4->CCR |= 0x01;
	system_delay_ms(10);
}

u8 devices_usart01_debug_read_ch(void)
{
	u8 ch;
	if( debug_rx_flag == 0)
		return 0;
	debug_rx_flag = 0;
	DMA1_Channel5->CCR &= ~0x01;
	ch = debug_rx_buf[0];
	DMA1_Channel5->CMAR = (u32) debug_rx_buf;
	DMA1_Channel5->CNDTR = DEBUG_BUFFER_LEN;
	DMA1_Channel5->CCR |= 0x01;
	return ch;
}

u16 devices_usart01_debug_read_buf(u16 size, u8 * buf)
{
	u16 i, len;
	//判断是否接收到数据，未接收到则返回0
	if( debug_rx_flag == 0)
		return 0;
	//清除标记位
	debug_rx_flag = 0;
	DMA1_Channel5->CCR &= ~0x01;
	//计算接收到的数据长度
	len = DEBUG_BUFFER_LEN - DMA1_Channel5->CNDTR;
	if( len > size)
		len = size-1;
	for( i = 0;i < len;i ++)
		buf[i] = debug_rx_buf[i];
	buf[i] = 0;
	DMA1_Channel5->CMAR = (u32) debug_rx_buf;
	DMA1_Channel5->CNDTR = DEBUG_BUFFER_LEN;
	DMA1_Channel5->CCR |= 0x01;
	return len;
}

//==============串口1中断服务函数========
void USART1_IRQHandler(void)
{
	u8 temp;
	if( USART1->SR & (1<<4))
	{
		temp = USART1->DR;
		debug_rx_flag = temp;
		debug_rx_flag = 1;
	}
}
