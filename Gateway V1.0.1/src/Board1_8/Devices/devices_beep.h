/***
 * \file    devices_beep.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   ·äÃùÆ÷Ä£¿éÇý¶¯
*/
#ifndef DEVICES_BEEP_H
#define DEVICES_BEEP_H

#include "system_option.h"

#define BEEP_CONTROL	PAxOut(15)				//·äÃùÆ÷¿ØÖÆÒý½Å

void devices_beep_init(void);

#endif
