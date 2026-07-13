#include "function.h"

#if  APP_BOARD_ID ==1u
#include "devices_pt4115.h"
#include "devices_ws2812b.h"
#include "system_delay.h"
void devices_init() {
    devices_pt4115_init(255, 72-1);
    devices_ws2812b_init();
}
void app_dev_procotol(app_agreement_pt p) {
    if(p->addr==0x10) {
        if(p->device==0x11) {
            if(p->data_len==10) {
                devices_pt4115_change(p->data[0],0,0,0);
            }
        } else if(p->device==0x12) {
            if(p->data_len==10) {
                devices_pt4115_change(0,p->data[0],0,0);
            }
        } else if(p->device==0x13) {
            if(p->data_len==10) {
                devices_pt4115_change(0,0,p->data[0],0);
            }
        } else if(p->device==0x14) {
            if(p->data_len==10) {
                devices_pt4115_change(0,0,0,p->data[0]);
            }
        } else if(p->device==0x15) { //炫彩LED灯
					//bit0-7蓝色 bit8-15绿色 bit16-23红色 //g R B 
            if(p->data_len==12) {
                u8 g = p->data[0];
                u8 r = p->data[1];
                u8 b = p->data[2];
                u32 rgb_value= b | r<<8 | g<<16;
                devices_ws2812b_pixel_en(0,0,16,&rgb_value,0);
            }
        }
    }

}
#endif

#if  APP_BOARD_ID ==2u
#include "devices_sht20.h"
#include "devices_co2.h"
#include "devices_software_iic.h"
#include "devices_veml6070.h"
#include "devices_bh1750.h"
#include "system_delay.h"
#include "devices_usart02_zigbee.h"
#include "devices_usart01_debug.h"
void devices_init() {
    //节点板2设备初始化函数调用
    devices_co2_init(); //CO2
    devices_software_iic_init();   //IIC
    devices_vm6070_set(); //紫外线
    devices_bh1750_init();//光照强度
}
void app_dev_procotol(app_agreement_pt p) {
    u8 buf[20]= {0XEF,0XEF,0X00,0XFF,0XFF};
    u16 zw_data;
    u16 tpt_data;
    u16 hum_data;
    u16 gz_data;
		float temp;
		float hum;
    if(p->addr==0x20) { //fe fe 00 ff ff 09 20 16 ff
        if(p->device==0x16) {
            u16 co2_data=devices_co2_measure();
            //发送数据 EF EF 00 FF FF 0B 20 16 co2_data FF
            buf[5]=0x0b;
            buf[6]=0x20;
            buf[7]=0x16;
            buf[8]=co2_data&0xff;
            buf[9]=co2_data>>8;
            buf[10]=0xFF;
           devices_usart02_zigbee_send_buf(11,buf);
					//devices_usart01_debug_send_buf(11, buf);
						
        } else if(p->device==0x17) { //fe fe 00 ff ff 09 20 17 ff
            devices_vm6070_measure(&zw_data);//数据值
            //发送数据 EF EF 00 FF FF 0B 20 16 gz_data FF
            buf[5]=0x0b;
            buf[6]=0x20;
            buf[7]=0x17;
            buf[8]=zw_data&0xff;
            buf[9]=zw_data>>8;
            buf[10]=0XFF;
            devices_usart02_zigbee_send_buf(11,buf);
        } else if(p->device==0x18) { //温度
            devices_sht20_original_temp(&tpt_data);
            buf[5]=0x0b;
            buf[6]=0x20;
            buf[7]=0x18;
					  temp = ((float)tpt_data * 0.00268127)-46.85;
						buf[8]=temp;
					  buf[9]=(temp-buf[8])*100;
//            buf[8]=tpt_data>>8;
//            buf[9]=tpt_data&0xff;
            buf[10]=0XFF;
            devices_usart02_zigbee_send_buf(11,buf);
        } else if(p->device==0x19) {
            devices_sht20_original_humi(&hum_data);
            buf[5]=0x0b;
            buf[6]=0x20;
            buf[7]=0x19;
//            buf[8]=tpt_data>>8;
//            buf[9]=tpt_data&0xff;
					 hum=((float)hum_data * 0.00190735)-6;
					 buf[8]=hum;
					 buf[9]=(hum-buf[8])*100;
           buf[10]=0XFF;
            devices_usart02_zigbee_send_buf(11,buf);
        } else if(p->device==0x2C) {
            devices_bh1750_original_measure(&gz_data);
            buf[5]=0x0b;
            buf[6]=0x20;
            buf[7]=0x2C;
					  gz_data*=0.6;//误差
            buf[8]=gz_data&0xff;
            buf[9]=gz_data>>8;
            buf[10]=0XFF;
            devices_usart02_zigbee_send_buf(11,buf);
        }
    }
}
#endif
#if  APP_BOARD_ID ==3u
//甲烷、火光、烟雾、红外传感器
//FE FE 00 FF FF 09 30 1X FF 设备正常收到0x01 异常0x02
#include "devices_environment.h"
void devices_init() {
    devices_environment_init();
}
void app_dev_procotol(app_agreement_pt p) {
    u8 buf[20]= {0XEF,0XEF,0X00,0XFF,0XFF};
    if(p->addr==0x30) {
        if(p->device==0x1A) { //甲烷
            buf[5]=0x0A;
            buf[6]=0x30;
            buf[7]=0x1A;
            buf[9]=0xFF;
            if(!Methane_Measure()) {
                buf[8]=0x02;
                devices_usart02_zigbee_send_buf(10,buf);
            } else {
                buf[8]=0x01;
                devices_usart02_zigbee_send_buf(10,buf);
            }
        } else if(p->device==0x1B) { //火光
            buf[5]=0X0A;
            buf[6]=0x30;
            buf[7]=0X1B;
            buf[9]=0xFF;
            if(!Fire_Measure()) {
                buf[8]=0x02;

                devices_usart02_zigbee_send_buf(10,buf);
            } else {
                buf[8]=0x01;

                devices_usart02_zigbee_send_buf(10,buf);
            }
        } else if(p->device==0x1C) { //烟雾
            buf[5]=0X0A;
            buf[6]=0x30;
            buf[7]=0X1C;
            buf[9]=0xFF;
            if(!Smog_Measure()) {
                buf[8]=0x02;

                devices_usart02_zigbee_send_buf(10,buf);
            } else {
                buf[8]=0x01;
                devices_usart02_zigbee_send_buf(10,buf);
            }
        } else if(p->device==0x1D) { //人体红外
            buf[5]=0X0A;
            buf[6]=0x30;
            buf[7]=0X1D;
            buf[9]=0xFF;
            if(!Infrared_Measure()) {
                buf[8]=0x02;
                devices_usart02_zigbee_send_buf(10,buf);
            } else {
                buf[8]=0x01;
                devices_usart02_zigbee_send_buf(10,buf);
            }
        }
    }
}
#endif
#if  APP_BOARD_ID ==4u
#include "devices_pm2_5.h"
#include "devices_usart03_device.h"
#include "devices_sht20.h"
#include "devices_software_iic.h"
void devices_init() {
    devices_pm2_5_init();
    devices_software_iic_init();
}
void app_dev_procotol(app_agreement_pt p) {
    u8 buf[20]= {0XEF,0XEF,0X00,0XFF,0XFF};
    u16 pm_data;
    u16 tpt_data;
    u16 hum_data;
		float temp;
		float hum;
    buf[5]=0x0b;
    buf[6]=0x40;
    buf[10]=0XFF;
    if(p->addr==0x40) {
        if(p->device==0x1E) { //PM2.5
            pm_data=devices_pm2_5_measure();
            buf[7]=0x1E;
            buf[8]=pm_data&0xff;
            buf[9]=pm_data>>8;
            devices_usart02_zigbee_send_buf(11,buf);

        } else if(p->device==0x1F) { //温度
            devices_sht20_original_temp(&tpt_data);
            buf[7]=0x1F;
            temp = ((float)tpt_data * 0.00268127)-46.85;
						buf[8]=temp;
					  buf[9]=(temp-buf[8])*100;
            devices_usart02_zigbee_send_buf(11,buf);

        } else if(p->device==0x20) {
            devices_sht20_original_humi(&hum_data);
            buf[7]=0x20;
            hum=((float)hum_data * 0.00190735)-6;
					  buf[8]=hum;
					  buf[9]=(hum-buf[8])*100;
            devices_usart02_zigbee_send_buf(11,buf);

        }
    }
}
#endif
#if  APP_BOARD_ID ==5u
#include "devices_security.h"
void devices_init() {
    devices_security_init();
}
void app_dev_procotol(app_agreement_pt p) {
    u8 buf[20]= {0XEF,0XEF,0X00,0XFF,0XFF};
    buf[5]=10;
    buf[6]=0x50;
    buf[9]=0XFF;
    if(p->addr==0x50) {
        if(p->device==0x21) { //报警器
            if(p->data[0]==0x01) { //开启
                WARN_BEEP=1;
            } else if(p->data[0]==0x02) {
                WARN_BEEP=0;
            }
        } else if(p->device==0x22) {
            if(p->data[0]==0x01) { //开启
                WARN_LED=1;
            } else if(p->data[0]==0x02) {
                WARN_LED=0;    //关闭
            }
        } else if(p->device==0x23) { //对射开关
            if(!SWITCH_RELATIVE()) { //触发发送0x02 
                buf[7]=0x23;
                buf[8]=0x02;
                devices_usart02_zigbee_send_buf(10,buf);
            } else {
                buf[7]=0x23;
                buf[8]=0x01;
                devices_usart02_zigbee_send_buf(10,buf);

            }
        } else if(p->device==0x24) { //反射开关
            if(!SWITCH_CONTRARY()) {
                buf[7]=0x24;
                buf[8]=0x02;
                devices_usart02_zigbee_send_buf(10,buf);
            } else {
                buf[7]=0x24;
                buf[8]=0x01;
                devices_usart02_zigbee_send_buf(10,buf);

            }
        }
    }
}
#endif
#if  APP_BOARD_ID ==6u
#include "devices_servo.h"
void devices_init() {
    devices_servo_init(20000, 72-1);
}
void app_dev_procotol(app_agreement_pt p) { //舵机//0x25//0x01打开，0x02关闭
    if(p->addr==0x60) {
        if(p->device==0x25) {
            if(p->data[0]==0x01) {
                devices_servo_on();//打开
            } else if(p->data[0]==0x02) {
                devices_servo_off();
            }
        }
    }
}
#endif
#if  APP_BOARD_ID ==7u
#include "devices_step_motor.h"
#include "devices_fan.h"
#include "devices_relay.h"
void devices_init() {
    delays_step_motor_init();
    devices_fan_init();
    devices_relay_init();
}
//步进电机//0x26//风扇//0x27//继电器1//0x28//继电器2// 0X29
//0x01打开，0x02关闭
void app_dev_procotol(app_agreement_pt p) {
    if(p->addr==0x70) {
        if(p->device==0x26) {
            if(p->data[0]==0x01) {
                delays_step_motor_control(2000,150,0);
            } else if(p->data[0]==0x02) {
                delays_step_motor_control(2000,150,1);
            }
        } else if(p->device==0x27) {
            if(p->data[0]==0x01) {
                FAN_Set=1;
            } else if(p->data[0]==0x02) {
                FAN_Set=0;
            }
        } else if(p->device==0x28) {
            if(p->data[0]==0x01) {
                Relay1_Set=1;
            } else if(p->data[0]==0x02) {
                Relay1_Set=0;
            }
        } else if(p->device==0x29) {
            if(p->data[0]==0x01) {
                Relay2_Set=1;
            } else if(p->data[0]==0x02) {
                Relay2_Set=0;
            }
        }
    }
}
#endif
#if  APP_BOARD_ID ==8u
#include "devices_lock.h"
void devices_init() {
    devices_lock_init();
}
void app_dev_procotol(app_agreement_pt p) {
    if(p->addr==0x80) {
        if(p->device==0x2A) {
            if(p->data[0]==0x01) {
                LOCK_CONTROL=1;
            } else if(p->data[0]==0x02) {
                LOCK_CONTROL=0;
            }
        }
    }

}
#endif
#if  APP_BOARD_ID ==9u
void app_dev_procotol(app_agreement_pt p) {

}
#endif
