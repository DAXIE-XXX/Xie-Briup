/***
 * \file    devices_co2.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   二氧化碳传感器驱动
*/
#ifndef DEVICES_CO2_H
#define DEVICES_CO2_H

#include "system_option.h"
#include "devices_usart03_device.h"

void devices_co2_init(void);
u16 devices_co2_measure(void);

#endif
