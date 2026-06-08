#include "dfu_main.h"
#include "usbd_cdc_if.h"
#include "Periph/FS_Serial.h"
#include "cmsis_os.h"
#include "bsp_devices.h"

extern void system_reset(void);

void dfu_main(int argc, char **argv)
{
    if (argc > 0) {
        USB_printf("dfu: too many arguments\n");
        return;
    }
    USB_printf("Saving filesystem and entering DFU mode...\n");
    FS_Serialize();
    osDelay(500);

    board_rtc_set_dfu_flag();

    system_reset();
}
