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
*/
#ifndef DEVICES_STEP_MOTOR_H
#define DEVICES_STEP_MOTOR_H

#include "system_option.h"
//IO操作函数	 
#define A4988_EN	  	PBout(0) 
#define A4988_MS1    	PBout(1) 	 
#define A4988_MS2	  	PBout(5) 
#define A4988_MS3    	PBout(6) 
#define A4988_RESET	    PBout(7) 
#define A4988_SLEEP     PBout(8) 
#define A4988_DIR	  	PBout(13) 

/*********************************************************
名称：delays_step_motor_control(u16 cycle, u16 pulse_num)
说明：生成指定个数脉冲，每个脉冲周期为cycle微秒，脉冲个数生成的个数
      和单脉冲高电平时间有关系，脉冲个数就由高电平时间来确定
参数cycle：为TIM3一个脉冲周期,单位(us)
参数pulse_num：为脉冲个数，决定步进电机步数
返回值：无
*********************************************************/
void delays_step_motor_control(u16 cycle, u16 pulse_num, u8 dir);

/******************************************
函数名：delays_step_motor_init
函数功能：初始化A4988步进电机驱动芯片
*******************************************/
void  delays_step_motor_init(void);


#endif
