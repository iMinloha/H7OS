#include "save_main.h"
#include "usbd_cdc_if.h"
#include "Periph/FS_Serial.h"

void save_main(int argc, char **argv){
    if(argc > 0) {
        USB_printf("save: too many arguments\n");
        return;
    }
    FS_Serialize();
}