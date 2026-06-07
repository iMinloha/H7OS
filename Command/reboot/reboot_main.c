#include "reboot_main.h"
#include "usbd_cdc_if.h"
#include "Periph/FS_Serial.h"
#include "cmsis_os.h"

extern void system_reset(void);

void reboot_main(int argc, char **argv){
    if(argc > 0) {
        USB_printf("reboot: too many arguments");
        return;
    }
    USB_printf("Saving filesystem before reboot...\n");
    FS_Serialize();
    osDelay(1000);
    system_reset();
}
