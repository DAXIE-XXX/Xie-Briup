#include "agreement.h"
#include "devices_usart02_zigbee.h"
#include "devices_usart01_debug.h"
app_agreement aa;
u8 down_buf[20]= {0};
u8 up_buf[20]= {0};
void analysis(u16 r_len,u8 *buf) {
    int i=0;
    //1.判断一下buf中保存的数据是否是一针完整数据
    if(buf[0]==0XFE&&buf[1]==0XFE&&
            buf[r_len-1]==0XFF) {
        aa.addr=buf[6];
        aa.device=buf[7];
        aa.data_len=buf[5];
        for(; i<buf[5]-9; i++) {
            aa.data[i]=buf[8+i];
        }
        //解析指令
        app_dev_procotol(&aa);
    }
}

