/***
 * \file    devices_usart02_zigbee.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-23
 * 
 * \brief   Zigbee串口驱动模块
 * 
*/
#include "devices_usart02_zigbee.h"

u8 zigbee_rx_buf[ZIGBEE_BUFFER_LEN] = {0};	//接收缓冲区
u8 zigbee_rx_flag = 0;			//收到数据标志

void devices_usart02_zigbee_init( u8 sclk, u32 baud)
{
    float temp;
    u16 mantissa,fraction;

    temp = (float)(sclk * 1000000)/(baud * 16);
    mantissa = temp;
    fraction = (temp - mantissa)*16;
    mantissa = (mantissa<<4) + fraction;

    RCC->APB2ENR |= 1<<2;
    GPIOA->CRL &= 0xffff000f;
    GPIOA->CRL |= 0x00008b20;

    ZIGBEE_RESET();//复位ZigBee

    RCC->APB1RSTR |= (1<<17);
    RCC->APB1RSTR &= ~(1<<17);
    RCC->APB1ENR |= (1<<17);

    USART2->BRR = mantissa;//设置波特率

    USART2->CR1 |= (1<<13)|(1<<4)|(1<<3)|(1<<2);//IDLE INTERRUPT
    USART2->CR3 |= (1<<6)|(1<<7);//DMAR DMAT

    RCC->AHBENR |= (1<<0);//DMA1 ENR
    system_delay_ms(5);

    DMA1_Channel7->CCR &= ~(0XFFFFFFFF); 
    DMA1_Channel7->CNDTR = 0;
    DMA1_Channel7->CPAR = (u32)&(USART2->DR);
    DMA1_Channel7->CMAR = 0;
    DMA1_Channel7->CCR |= (1<<7)|(1<<4);
    //中优先级 存储器地址增量 从存储器读 传输完成中断

    DMA1_Channel6->CCR &= ~(0XFFFFFFFF);
    DMA1_Channel6->CPAR = (u32)&(USART2->DR);
    DMA1_Channel6->CMAR = (u32)(zigbee_rx_buf);
    DMA1_Channel6->CNDTR = ZIGBEE_BUFFER_LEN;
    DMA1_Channel6->CCR |= (1<<13)|(1<<12)|(1<<7)|(1<<0);
    //最高优先级 存储器地址增量 从外设读 允许传输完成中断 启动传输通道

    USART2->SR |= 0x1ff;
    system_option_nvic_priority_set(1,2,USART2_IRQn);
}

void devices_usart02_zigbee_send_ch(u8 ch)
{
	DMA1_Channel7->CCR &= ~0x01;
	DMA1_Channel7->CMAR = (u32) &ch;
	DMA1_Channel7->CNDTR = 1;
	DMA1_Channel7->CCR |= 0x01;
}

void devices_usart02_zigbee_send_str(char * str)
{
	u16 len = strlen( str);
	DMA1_Channel7->CCR &= ~0x01;
	DMA1_Channel7->CMAR = (u32) str;
	DMA1_Channel7->CNDTR = len;
	DMA1_Channel7->CCR |= 0x01;
}

void devices_usart02_zigbee_send_buf(u16 len, u8 * buf)
{
	//关闭U1.Tx对应的DMA1.CH4通道
	DMA1_Channel7->CCR &= ~0x01;
	//设置要发送的数据的内存首地址
	DMA1_Channel7->CMAR = (u32) buf;
	//设置要发送的数据数量
	DMA1_Channel7->CNDTR = len;
	//打开通道
	DMA1_Channel7->CCR |= 0x01;
}

void devices_usart02_zigbee_send_number(double num)
{
	//取浮点数后两位小数的值，转化为无符号整型
	u32 number = num * 100;
	u8 point = 0, i, len;
	u8 buf[30] = {0};
	if( num == 0)
	{
		devices_usart02_zigbee_send_ch('0');
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
	
	DMA1_Channel7->CCR &= ~0x01;
	DMA1_Channel7->CMAR = (u32) buf;
	DMA1_Channel7->CNDTR = len;
	DMA1_Channel7->CCR |= 0x01;
}


u8 devices_usart02_zigbee_read_ch(void)
{
	u8 ch;
	if( zigbee_rx_flag == 0)
		return 0;
	zigbee_rx_flag = 0;
	DMA1_Channel6->CCR &= ~0x01;
	ch = zigbee_rx_buf[0];
	DMA1_Channel6->CMAR = (u32) zigbee_rx_buf;
	DMA1_Channel6->CNDTR = ZIGBEE_BUFFER_LEN;
	DMA1_Channel6->CCR |= 0x01;
	return ch;
}

u16 devices_usart02_zigbee_read_buf(u16 size, u8 * buf)
{
	u16 i, len;
	//判断是否接收到数据，未接收到则返回0
	if( zigbee_rx_flag == 0)
		return 0;
	//清除标记位
	zigbee_rx_flag = 0;
	DMA1_Channel6->CCR &= ~0x01;
	//计算接收到的数据长度
	len = ZIGBEE_BUFFER_LEN - DMA1_Channel6->CNDTR;
	if( len > size)
		len = size-1;
	for( i = 0;i < len;i ++)
		buf[i] = zigbee_rx_buf[i];
	buf[i] = 0;
	DMA1_Channel6->CMAR = (u32) zigbee_rx_buf;
	DMA1_Channel6->CNDTR = ZIGBEE_BUFFER_LEN;
	DMA1_Channel6->CCR |= 0x01;
	return len;
}

//==============串口2中断服务函数========
void USART2_IRQHandler(void)
{
	u8 temp;
	if( USART2->SR & (1<<4))
	{
		temp = USART2->DR;
		zigbee_rx_flag = temp;
		zigbee_rx_flag = 1;
	}
}
