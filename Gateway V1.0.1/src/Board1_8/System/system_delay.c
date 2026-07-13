#include "system_delay.h"

//每us和每ms的计数次数
u8 fac_us = 0;
u32 fac_ms = 0;

//延时模块初始化函数声明 (MHz)
void system_delay_init(u8 sclk)
{
	fac_us = sclk / 8;		//每微秒计数次数
	fac_ms = fac_us * 1000;	//每毫秒计数次数
}


void system_delay_us(u32 us)	//按微秒延时
{
	u32 value = us * fac_us;
	u32 temp = 0;
	
	SysTick->CTRL &= ~0x01;	//除能定时器
	SysTick->LOAD = value;	//设置计数值
	SysTick->VAL = 0;		//清空计数器
	SysTick->CTRL |= 0x01;	//使能定时器
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01) && !(temp&(1<<16)));//判断 第0位是1 为真
	
	SysTick->CTRL &= ~0x01;	//除能定时器
}

void system_delay_ms(u32 ms)	//按毫秒延时
{
	u32 value;
	u32 temp = 0;
	
	if(ms > 1800)
		ms = 1800;
	
	value = ms * fac_ms;
	
	SysTick->CTRL &= ~0x01;	//除能定时器
	SysTick->LOAD = value;	//设置计数值
	SysTick->VAL = 0;		//清空计数器
	SysTick->CTRL |= 0x01;	//使能定时器
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01) && !(temp&(1<<16)));//判断 第0位是1 为真
	
	SysTick->CTRL &= ~0x01;	//除能定时器
}

//按秒延时
void system_delay_sec( double sec)
{
	//1.通过sec计算毫秒ms数
	//2.循环判断ms是否大于1800
	//3.若大于，则延时1800后，减去1800ms
	//4.若ms小于1800,结束循环,延时剩余时长
	u32 ms = (u32)(sec*1000);
	while( ms >= 1800)
	{
		system_delay_ms(1800);
		ms -= 1800;
	}
	if( ms > 0)
		system_delay_ms(ms);
}

//按小时、分钟和秒延时的函数
void delayHMS(u8 hour, u8 min, u8 sec);

