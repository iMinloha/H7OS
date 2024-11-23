#include "reset_main.h"
#include "memctl.h"
#include "usbd_cdc_if.h"
#include "quadspi.h"
#include "init.d/CPU/CS.h"
#include "usart.h"

void reset_main(int argc, char *argv[]){
    char *input_buf = kernel_alloc(128);
    memset(input_buf, 0, 128);

    USB_color_printf(LIGHT_RED, "reset: Are you sure to reset the system? (y/n)\n");
    USB_scanf(input_buf);
    while (input_buf[0] == 0) USB_scanf(input_buf);

    if (strcmp(input_buf, "y") == 0){
        USB_color_printf(YELLOW, "reset: System is resetting...\n");
        if (QSPI_W25Qxx_ChipErase() == QSPI_W25Qxx_OK) {
            USB_color_printf(YELLOW, "reset: Flash chip erased.\n");
            CS_clean();
            USB_color_printf(YELLOW, "reset: It will take effect after restart\n");
        }
        else USB_color_printf(LIGHT_RED, "reset: Flash chip erase failed.\n");
    }else{
        USB_color_printf(LIGHT_GRAY, "reset: System reset canceled.\n");
    }
    kernel_free(input_buf);
}