/***
 * \file    devices_servo.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   舵机模块驱动
*/
#ifndef DEVICES_SERVO_H
#define DEVICES_SERVO_H

#include "system_option.h"

void devices_servo_init(u16 arr, u16 psc);
//闸机打开
void devices_servo_on(void);
//闸机关闭
void devices_servo_off(void);

#endif
