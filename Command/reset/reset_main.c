#include "reset_main.h"
#include "usbd_cdc_if.h"
#include "quadspi.h"
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
        if (QSPI_W25Qxx_ChipErase() == QSPI_W25Qxx_OK)
            USB_color_printf(YELLOW, "reset: done, reboot to take effect\n");
        else
            USB_color_printf(LIGHT_RED, "reset: erase failed\n");
    } else {
        USB_color_printf(LIGHT_GRAY, "reset: canceled\n");
    }
}
