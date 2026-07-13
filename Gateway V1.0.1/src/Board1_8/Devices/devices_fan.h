/***
 * \file    devices_fan.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   Mini风扇模块驱动
*/
#ifndef DEVICES_FAN_H
#define DEVICES_FAN_H

#include <stm32f10x.h>

#define FAN_Set		PAxOut(6)   //1风扇打开

void devices_fan_init(void);


#endif
