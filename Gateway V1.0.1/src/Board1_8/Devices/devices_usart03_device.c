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
 * \brief   设备串口模块
 * 
*/
#include "devices_usart03_device.h"

u8 device_rx_buf[DEVICE_BUFFER_LEN] = {0};	//接收缓冲区
u8 device_rx_flag = 0;			//收到数据标志

void devices_usart03_device_init( u8 sclk, u32 baud)
{
    float temp;
    u16 mantissa,fraction;

    temp = (float)(sclk * 1000000)/(baud * 16);
    mantissa = temp;
    fraction = (temp - mantissa)*16;
    mantissa = (mantissa<<4) + fraction;

    RCC->APB2ENR |= 1<<3;
    GPIOB->CRH &= 0xffff00ff;
    GPIOB->CRH |= 0x00008b00;

    RCC->APB1RSTR |= (1<<18);
    RCC->APB1RSTR &= ~(1<<18);
    RCC->APB1ENR |= (1<<18);

    USART3->BRR = mantissa;

    USART3->CR1 |= (1<<13)|(1<<4)|(1<<3)|(1<<2);//IDLE INTERRUPT
    USART3->CR3 |= (1<<6)|(1<<7);//DMAR DMAT

    RCC->AHBENR |= (1<<0);//DMA1 ENR
    system_delay_ms(5);

    DMA1_Channel2->CCR &= ~(0XFFFFFFFF); 
    DMA1_Channel2->CNDTR = 0;
    DMA1_Channel2->CPAR = (u32)&(USART3->DR);
    DMA1_Channel2->CMAR = 0;
    DMA1_Channel2->CCR |= (1<<7)|(1<<4);
    //中优先级 存储器地址增量 从存储器读 传输完成中断

    DMA1_Channel3->CCR &= ~(0XFFFFFFFF);//初始化CCR寄存器
    DMA1_Channel3->CPAR = (u32)&(USART3->DR);//串口3外设地址
    DMA1_Channel3->CMAR = (u32)(device_rx_buf);
    DMA1_Channel3->CNDTR = DEVICE_BUFFER_LEN;
    DMA1_Channel3->CCR |= (1<<13)|(0<<12)|(1<<7)|(1<<0);
    //高优先级 存储器地址增量 从外设读 循环模式 允许传输完成中断 启动传输通道

    system_option_nvic_priority_set(1,1,USART3_IRQn);
    USART3->SR = 0;
}

void devices_usart03_device_send_ch(u8 ch)
{
	DMA1_Channel2->CCR &= ~0x01;
	DMA1_Channel2->CMAR = (u32) &ch;
	DMA1_Channel2->CNDTR = 1;
	DMA1_Channel2->CCR |= 0x01;
}

void devices_usart03_device_send_str(char * str)
{
	u16 len = strlen( str);
	DMA1_Channel2->CCR &= ~0x01;
	DMA1_Channel2->CMAR = (u32) str;
	DMA1_Channel2->CNDTR = len;
	DMA1_Channel2->CCR |= 0x01;
}

void devices_usart03_device_send_buf(u16 len, u8 * buf)
{
	//关闭U1.Tx对应的DMA1.CH4通道
	DMA1_Channel2->CCR &= ~0x01;
	//设置要发送的数据的内存首地址
	DMA1_Channel2->CMAR = (u32) buf;
	//设置要发送的数据数量
	DMA1_Channel2->CNDTR = len;
	//打开通道
	DMA1_Channel2->CCR |= 0x01;
}

void devices_usart03_device_send_number(double num)
{
	//取浮点数后两位小数的值，转化为无符号整型
	u32 number = num * 100;
	u8 point = 0, i, len;
	u8 buf[30] = {0};
	if( num == 0)
	{
		devices_usart03_device_send_ch('0');
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
	
	DMA1_Channel2->CCR &= ~0x01;
	DMA1_Channel2->CMAR = (u32) buf;
	DMA1_Channel2->CNDTR = len;
	DMA1_Channel2->CCR |= 0x01;
	system_delay_ms(10);
}

u8 devices_usart03_device_read_ch(void)
{
	u8 ch;
	if( device_rx_flag == 0)
		return 0;
	device_rx_flag = 0;
	DMA1_Channel3->CCR &= ~0x01;
	ch = device_rx_buf[0];
	DMA1_Channel3->CMAR = (u32) device_rx_buf;
	DMA1_Channel3->CNDTR = DEVICE_BUFFER_LEN;
	DMA1_Channel3->CCR |= 0x01;
	return ch;
}

u16 devices_usart03_device_read_buf(u16 size, u8 * buf)
{
	u16 i, len;
	//判断是否接收到数据，未接收到则返回0
	if( device_rx_flag == 0)
		return 0;
	//清除标记位
	device_rx_flag = 0;
	DMA1_Channel3->CCR &= ~0x01;
	//计算接收到的数据长度
	len = DEVICE_BUFFER_LEN - DMA1_Channel3->CNDTR;
	if( len > size)
		len = size-1;
	for( i = 0;i < len;i ++)
		buf[i] = device_rx_buf[i];
	buf[i-1] = 0;
	DMA1_Channel3->CMAR = (u32) device_rx_buf;
	DMA1_Channel3->CNDTR = DEVICE_BUFFER_LEN;
	DMA1_Channel3->CCR |= 0x01;
	return len;
}

//==============串口3中断服务函数========
void USART3_IRQHandler(void)
{
	u8 temp;
	if( USART3->SR & (1<<4))
	{
		temp = USART3->DR;
		device_rx_flag = temp;
		device_rx_flag = 1;
	}
}
