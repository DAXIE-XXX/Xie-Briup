/***
 * \file    devices_security.h
 * 
 * \copyright   Copyright [2020] by Briup .
 *              All rights reserved. This software and code comprise proprietary
 *              information of Briup. This software and code may not be reproduced,
 *              used, altered, reengineered, distributed or disclosed to others
 *              without the written consent of Briup.
 * \create  John.Chen   2020-11-30
 * 
 * \brief   安防节点驱动
 *          文件功能：智能门窗相关传感器驱动配置
 *          管脚连接：
 *          				报警器		-->PB8
 *          				报警灯		-->PB9
 *          				光电开关	-->PB0
 *          				对射开关	-->PB1
*/
#ifndef DEVICES_SECURITY
#define DEVICES_SECURITY

#include "system_option.h"


#define WARN_BEEP           PBxOut(8)	//报警器
#define WARN_LED            PBxOut(9)	//报警LED
#define SWITCH_CONTRARY()   PBxIn(0)	//反射开关
#define SWITCH_RELATIVE()   PBxIn(1)	//对射开关

void devices_security_init(void);


#endif
