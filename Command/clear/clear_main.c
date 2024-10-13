#include "clear_main.h"
#include "usbd_cdc_if.h"
#include "usart.h"

void clear_main(int argc, char **argv){
    if (argc > 0) {
        USB_printf("clear: too many arguments");
        return;
    }
    USB_printf(CLEAR);
}