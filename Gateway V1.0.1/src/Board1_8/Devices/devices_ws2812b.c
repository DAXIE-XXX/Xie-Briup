/***
 * \file    devices_ws2812b.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   炫彩LED灯模块驱动
 *          器件型号：KS6812 (WS2812B与之操作类似)
 *          通信方式：SPI(单线发送)+DMA
 *          文件功能：控制KS6812显示
 *          传输数据顺序：G R  B 即先传出绿色 再传出红色 再传出蓝色
*/

#include "devices_ws2812b.h"

#include "system_delay.h"
u16 PixelBuffer[1637];//100+64*24+1 0~99永远为0 用于初始化 1636永远为0 用于传输完成拉低总线

void devices_ws2812b_init()
{
	u16 i;
	//SPI配置
	
	RCC->APB2ENR|=1<<12;//SPI1时钟使能 
	RCC->APB2ENR|=1<<2; //PROTA时钟使能
	RCC->AHBENR|=1<<0;  //DMA1时钟使能
	
	GPIOA->CRL&=0X0FFFFFFF;		 
	GPIOA->CRL|=0XB0000000;		 //PA7 复用 
	
	//GPIOA->ODR|=0X7<<5;			 //初始化高电平
	
	SPI1->CR1|=1<<15;
	SPI1->CR1|=1<<14;//单线只发模式
	SPI1->CR1|=1<<11;//16位数据格式
	//SPI1->CR1|=1<<10;
	SPI1->CR1|=1<<9; //软件nss管理
	SPI1->CR1|=1<<8;  
	SPI1->CR1|=0<<7; //MSBfirst 
	SPI1->CR1|=1<<3; //Fsck=Fcpu/4
	SPI1->CR1|=1<<2; //SPI主机
	SPI1->CR1&=~0x02;//空闲模式下SCK为0 CPOL=0
	SPI1->CR1&=~0x01;//数据采样从第一个时间边沿开始,CPHA=0  

  SPI1->CR2|=1<<1;//启动发送缓冲区DMA 
	SPI1->CR1|=1<<6; //SPI设备使能
	
	DMA1_Channel3->CCR|=1<<12;//中优先级
    DMA1_Channel3->CCR|=1<<10;//存储器数据宽度为16位
	DMA1_Channel3->CCR|=1<<8;//外设数据宽度为16位
	DMA1_Channel3->CCR|=1<<7;//存储器地址增量模式
	DMA1_Channel3->CCR|=1<<4;//从存储器读
	DMA1_Channel3->CNDTR=0;//传输数据量为0
	DMA1_Channel3->CPAR = (u32)&(SPI1->DR);//外设地址
	DMA1_Channel3->CMAR = (u32)(PixelBuffer);//存储器地址
	for(i=0;i<1536;i++)
	{
		PixelBuffer[i+100]=0X0000;
	}
    devices_ws2812b_clear();
}

void devices_ws2812b_en()
{
	DMA1_Channel3->CCR&=~(1<<0);//关闭DMA1通道3
	DMA1_Channel3->CNDTR = 1637;//初始化计数器的值
	DMA1_Channel3->CCR|=1<<0;//使能DMA传输
}
/*********************************************
函数名     devices_ws2812b_pixel_en
函数功能：根据起始坐标点点亮LED
输入参数: x,y为起始坐标点
					len：以起始坐标点开始（包括）要点亮LED的个数。
					RGB_Value:将要显示的颜色 bit0-7蓝色 bit8-15绿色 bit16-23红色
					mode：显示模式 
*********************************************/
void devices_ws2812b_pixel_en(u8 x,u8 y,u8 len,u32* RGB_Value,u8 mode)
{
	u16 Pixel_Point = 0;
	u16 One=0XFFC0;
	u16 Zero=0XF000;
	u32 i=0;
	Pixel_Point = (Y_LEN*y+x)*24+100;
	while(len--)
	{
		for(i=0X800000;i>=0X01;i>>=1)
		{
			PixelBuffer[Pixel_Point]= (*RGB_Value)&i ? One : Zero;
			Pixel_Point++;
		}
		if(mode == 1)
			RGB_Value++;
	}
	devices_ws2812b_en();
}
/*********************************************
函数名     devices_ws2812b_clear
函数功能： 熄灭所有的LED
*********************************************/
void devices_ws2812b_clear(void)
{
	u16 i;
	for(i=0;i<1536;i++)
	{
		PixelBuffer[i+100]=0XF000;
	}
	devices_ws2812b_en();
}
