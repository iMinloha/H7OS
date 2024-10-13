#include "reboot_main.h"
#include "usbd_cdc_if.h"

extern void system_reset(void);

void reboot_main(int argc, char **argv){
    if(argc > 0) {
        USB_printf("reboot: too many arguments");
        return;
    }
    system_reset();
}