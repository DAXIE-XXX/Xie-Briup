#include "config.h"
#include "system_delay.h"
#include "system_option.h"
#include "function.h"
#include "agreement.h"
#include "devices_usart02_zigbee.h"
#include "devices_usart01_debug.h"
#if  APP_BOARD_ID==0u
int main() {
    while(1);
}
#endif
#if APP_BOARD_ID<9u
int main() {
    u8 buf[200]= {0};
    u16 r_len=0;
    system_delay_init(72);
    system_option_jtag_set(SWD_ENABLE);
    devices_usart02_zigbee_init(36,57600);
	 devices_usart01_debug_init( 72, 57600);
    devices_init();
    while(1) {
        // 侦听并解析命令
       r_len=devices_usart02_zigbee_read_buf(100,buf);
			// r_len=devices_usart01_debug_read_buf(100, buf);
        if(r_len != 0) {
            if(buf[0]==0xFE && buf[1]==0xFE) { //1.判断数据的完整性
                analysis(r_len,buf);
            }
        }
    }
}
#endif
