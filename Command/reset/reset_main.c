/**
 * @file    reset_main.c
 * @brief   reset 命令 — 擦除 QSPI Flash 数据 (清除持久化)
 *          reset command — erase QSPI Flash data (clear persistence)
 */

#include "reset_main.h"
#include "usbd_cdc_if.h"
#include "platform.h"
#include <string.h>

void reset_main(int argc, char *argv[])
{
    char input_buf[8];

    USB_color_printf(LIGHT_RED, "reset: erase all data? (y/n)\n");
    USB_scanf(input_buf);
    while (input_buf[0] == 0)
        USB_scanf(input_buf);

    if (strcmp(input_buf, "y") == 0) {
        USB_color_printf(YELLOW, "reset: erasing flash...\n");
        if (Platform_QSPI_ChipErase() == 0)
            USB_color_printf(YELLOW, "reset: done, reboot to take effect\n");
        else
            USB_color_printf(LIGHT_RED, "reset: erase failed\n");
    } else {
        USB_color_printf(LIGHT_GRAY, "reset: canceled\n");
    }
}
