/**
 * @file    dfu_main.c
 * @brief   dfu 命令 — 进入 STM32 系统 bootloader (USB DFU)
 *          dfu command — enter STM32 system bootloader (USB DFU)
 */

#include "dfu_main.h"
#include "usbd_cdc_if.h"
#include "Periph/FS_Serial.h"
#include "cmsis_os.h"
#include "platform.h"

void dfu_main(int argc, char **argv)
{
    if (argc > 0) {
        USB_printf("dfu: too many arguments\n");
        return;
    }
    USB_printf("Saving filesystem and entering DFU mode...\n");
    FS_Serialize();
    osDelay(500);

    /* Platform_DFU_Request: 设置 RTC 标记 + 系统复位 */
    /* Platform_DFU_Request: set RTC flag + system reset */
    Platform_DFU_Request();
}
