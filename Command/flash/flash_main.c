/**
 * @file    flash_main.c
 * @brief   flash 命令 — 擦除 QSPI Flash
 *          flash command — erase QSPI Flash
 */

#include "flash_main.h"
#include "usbd_cdc_if.h"
#include "platform.h"

void flash_main(int argc, char **argv){
    if (argc > 0) {
        USB_printf("Usage: flash\n");
        return;
    }
    USB_printf("Erasing QSPI flash...\n");
    if (Platform_QSPI_ChipErase() == 0) {
        USB_printf("Flash erased.\n");
    } else {
        USB_color_printf(LIGHT_RED, "Flash erase failed!\n");
    }
}
