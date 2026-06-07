#include "flash_main.h"
#include "quadspi.h"
#include "usart.h"
#include "usbd_cdc_if.h"

void flash_main(int argc, char **argv){
    if (argc > 0) {
        USB_printf("Usage: flash\n");
        return;
    }
    USB_printf("Erasing QSPI flash...\n");
    if (QSPI_W25Qxx_ChipErase() == QSPI_W25Qxx_OK) {
        USB_printf("Flash erased.\n");
    } else {
        USB_color_printf(LIGHT_RED, "Flash erase failed!\n");
    }
}
