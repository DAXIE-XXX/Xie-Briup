/***
 * \file    devices_software_iic.h
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
#ifndef DEVICES_SOFTWARE_IIC_H
#define DEVICES_SOFTWARE_IIC_H
#include "system_option.h" 

//IO方向设置
#define SDA_IN()  {GPIOB->CRL&=0XFFFF0FFF;GPIOB->CRL|=8<<12;}	//PB3输入模式
#define SDA_OUT() {GPIOB->CRL&=0XFFFF0FFF;GPIOB->CRL|=3<<12;} //PB3输出模式
//IO操作函数	 
#define IIC_SCL    PBout(4) //SCL
#define IIC_SDA    PBout(3) //SDA
#define READ_SDA   PBin(3)  //输入SDA 

//IIC所有操作函数
void devices_software_iic_init(void);                //初始化IIC的IO口				 
void devices_software_iic_start(void);				//发送IIC开始信号
void devices_software_iic_stop(void);	  			//发送IIC停止信号
void devices_software_iic_send_byte(u8 txd);			//IIC发送一个字节
u8 devices_software_iic_read_byte(unsigned char ack);//IIC读取一个字节
u8 devices_software_iic_wait_ack(void); 				//IIC等待ACK信号
void devices_software_iic_ack(void);					//IIC发送ACK信号
void devices_software_iic_nack(void);				//IIC不发送ACK信号 
u8 devices_software_iic_single_write(unsigned char SlaveAddress,unsigned char REG_Address,unsigned char REG_Date);//单字节写入
unsigned char devices_software_iic_single_read(unsigned char SlaveAddress,unsigned char REG_Address);//单字节读取
unsigned char devices_software_iic_single_read_Len(unsigned char SlaveAddress,unsigned char REG_Address,u8 len,u8* buf);//多字节读取
u8 devices_software_iic_single_write_nd(u8 addr, u8 reg);
u8 devices_software_iic_single_read_nd(u8 addr, u8 len, u8 *buf);
#endif
















