#include "save_main.h"
#include "usbd_cdc_if.h"
#include "init.d/CPU/CS.h"
#include "usart.h"

// CS_save();

void save_main(int argc, char **argv){
    if(argc > 0) {
        USB_printf("save: too many arguments");
        return;
    }
    USB_color_printf(LIGHT_RED, "Saving in progress, please do not perform any operations\n");
    CS_save();
    USB_color_printf(LIGHT_GREEN, "Save Done!\n");
    // osDelay(1000);
    // system_reset();
}