#include "dfu_main.h"
#include "usbd_cdc_if.h"
#include "Periph/FS_Serial.h"
#include "cmsis_os.h"
#include "rtc.h"

/* DFU flag in RTC backup register 0 (survives system reset) */
#define RTC_BKP_DFU_FLAG    0x44465501

extern void system_reset(void);

void dfu_main(int argc, char **argv){
    if(argc > 0) {
        USB_printf("dfu: too many arguments\n");
        return;
    }
    USB_printf("Saving filesystem and entering DFU mode...\n");
    FS_Serialize();
    osDelay(500);

    /* Write DFU flag to RTC backup register (preserved across reset) */
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_BKP_DFU_FLAG);

    system_reset();
}
