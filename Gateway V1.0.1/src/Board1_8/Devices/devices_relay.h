/***
 * \file    devices_relay.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   继电器模块驱动
*/
#ifndef DEVICES_RELAY_H
#define DEVICES_RELAY_H

#include "system_option.h"

#define Relay1_Set			PAxOut(5)				//1常开灯亮 0关闭
#define Relay2_Set			PAxOut(4)

void devices_relay_init(void);



#endif
