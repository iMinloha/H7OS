#include <stdio.h>
#include "xShellTask.h"
#include "cmsis_os.h"
#include "RAMFS.h"
#include "TaskHead.h"
#include "torch_iic.h"
#include "ltdc.h"
#include "usbd_cdc_if.h"

// 全局任务
extern Task_t xShell;

// 触摸屏信息
extern volatile TouchStructure touchInfo;

/*
uint8_t buf[64];
memset(buf, 0, 64);
USB_scanf(buf);
 * */

void ShellTask(){

    osDelay(1000);
    uint8_t buf[64];
    memset(buf, 0, 64);
    while(1){
        TaskTickStart(xShell);
        execCMD("info /dev/Cortex-M7");
        // USB_printf("Hello World!\n");

        // buf[0] = 0;
        USB_scanf(buf);
        if (buf[0] != 0){

            USB_printf("You input: %s\n", buf);
            memset(buf, 0, 64);

        }

        osDelay(1000);
        TaskTickEnd(xShell);
    }
}