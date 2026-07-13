/***
 * \file    devices_lock.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   电磁锁模块驱动
*/
#ifndef DEVICES_LOCK_H
#define DEVICES_LOCK_H

#include "system_option.h"
#include "system_delay.h"

#define LOCK_CONTROL			PBxOut(9)				//电磁锁控制引脚

void devices_lock_init(void);




#endif
