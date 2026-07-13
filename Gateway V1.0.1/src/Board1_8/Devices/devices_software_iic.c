/***
 * \file    devices_software_iic.c
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   软件模拟IIC驱动
*/
#include "devices_software_iic.h"
#include "system_delay.h"

//初始化IIC
void devices_software_iic_init(void)
{					     
	RCC->APB2ENR|=1<<3;    //使能PORTB时钟	   	  
	GPIOB->CRL&=0XFFF00FFF;
	GPIOB->CRL|=0X00033000;;//PB3/PB4设置推挽输出 
	IIC_SCL=1;
	IIC_SDA=1;
}
//产生IIC起始信号
void devices_software_iic_start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	system_delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	system_delay_us(4);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void devices_software_iic_stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	system_delay_us(4);
	IIC_SCL=1; 
	system_delay_us(1);
	IIC_SDA=1;//发送I2C总线结束信号
	system_delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 devices_software_iic_wait_ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;system_delay_us(1);	   
	IIC_SCL=1;system_delay_us(1);	
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			//devices_software_iic_stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void devices_software_iic_ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	system_delay_us(2);
	IIC_SCL=1;
	system_delay_us(2);
	IIC_SCL=0;
}
//不产生ACK应答		    
void devices_software_iic_nack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	system_delay_us(2);
	IIC_SCL=1;
	system_delay_us(2);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答		  
void devices_software_iic_send_byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		system_delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL=1;
		system_delay_us(2); 
		IIC_SCL=0;	
		system_delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 devices_software_iic_read_byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        system_delay_us(2);
				IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		system_delay_us(1); 
    }	
		//IIC_SCL=0;	
    if (!ack)
        devices_software_iic_nack();//发送nACK
    else
        devices_software_iic_ack(); //发送ACK   
    return receive;
}
//单字节写入
u8 devices_software_iic_single_write(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char REG_Date)
{
	devices_software_iic_start();//发送起始信号
	devices_software_iic_send_byte(SlaveAddress);//发送设备地址
	if(devices_software_iic_wait_ack()){devices_software_iic_stop(); return 1;}
	devices_software_iic_send_byte(REG_Address);//发送操作寄存器地址
	devices_software_iic_wait_ack();
	devices_software_iic_send_byte(REG_Date);//发送写入寄存器的数据
	if(devices_software_iic_wait_ack()){devices_software_iic_stop(); return 1;}
	devices_software_iic_stop();
	system_delay_us(200);
	return 0;
}
//单字节读取
unsigned char devices_software_iic_single_read(unsigned char SlaveAddress,unsigned char REG_Address)
{
  unsigned char REG_Date;
	devices_software_iic_start();
	devices_software_iic_send_byte(SlaveAddress);
	if(devices_software_iic_wait_ack()){devices_software_iic_stop(); return 0;}
	devices_software_iic_send_byte(REG_Address);
	devices_software_iic_wait_ack();
	devices_software_iic_start();
	devices_software_iic_send_byte(SlaveAddress+1);
	devices_software_iic_wait_ack();
	REG_Date = devices_software_iic_read_byte(0);
	devices_software_iic_stop();
	return REG_Date;
}

//IIC连续读
//addr:器件地址
//reg:要读取的寄存器地址
//len:要读取的长度
//buf:读取到的数据存储区
//返回值:0,正常
//    其他,错误代码
unsigned char devices_software_iic_single_read_Len(unsigned char SlaveAddress,unsigned char REG_Address,u8 len,u8 *buf)
{ 
 	devices_software_iic_start(); 
	devices_software_iic_send_byte(SlaveAddress);//发送器件地址
	if(devices_software_iic_wait_ack())	//等待应答
	{
		devices_software_iic_stop();		 
		return 1;		
	}
    devices_software_iic_send_byte(REG_Address);	//写寄存器地址
    devices_software_iic_wait_ack();		//等待应答
		system_delay_ms(100);			//由于SHT20需要一定的时间进行数据测量，此处的延时不能少
    devices_software_iic_start();
	devices_software_iic_send_byte(SlaveAddress+1);//发送器件地址+读命令	
    devices_software_iic_wait_ack();		//等待应答 
	while(len)
	{
		if(len==1)*buf=devices_software_iic_read_byte(0);//读数据,发送nACK 
		else *buf=devices_software_iic_read_byte(1);		//读数据,发送ACK  
		len--;
		buf++; 
	}    
    devices_software_iic_stop();	//产生一个停止条件 
	return 0;	
}

u8 devices_software_iic_single_write_nd(u8 addr, u8 reg)
{
	devices_software_iic_start();//发送起始信号
	devices_software_iic_send_byte(addr);//发送设备地址
	if(devices_software_iic_wait_ack()){devices_software_iic_stop(); return 1;}
	devices_software_iic_send_byte(reg);//发送操作寄存器地址
	devices_software_iic_wait_ack();
	devices_software_iic_stop();
	system_delay_ms(5);
	return 1;
}

u8 devices_software_iic_single_read_nd(u8 addr, u8 len, u8 *buf)
{	
	devices_software_iic_start(); 
	devices_software_iic_send_byte(addr+1);//发送器件地址
	while(devices_software_iic_wait_ack())	//等待应答
	{
		devices_software_iic_stop();		 
		return 1;		
	}
	while(len)
	{
		if(len==1)*buf=devices_software_iic_read_byte(0);//读数据,发送nACK 
		else *buf=devices_software_iic_read_byte(1);		//读数据,发送ACK  
		len--;
		buf++; 
	}    
    devices_software_iic_stop();	//产生一个停止条件 
		system_delay_ms(5);
	return 1;
}





















