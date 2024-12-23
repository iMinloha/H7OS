#include <stdio.h>
#include "xShellTask.h"
#include "cmsis_os.h"
#include "RAMFS.h"
#include "TaskHead.h"
#include "torch_iic.h"
#include "usbd_cdc_if.h"
#include "memctl.h"
#include "quadspi.h"
#include "usart.h"
#include "usb_device.h"

/*** usb终端线程
 * @anchor Minloha
 * @brief 串口终端线程
 * @description 串口终端线程，用于接收串口指令并执行
 *              串口指令就是CMD_t链
 * */

// 全局任务
extern Task_t xShell;

// 触摸屏信息
extern volatile TouchStructure touchInfo;

/*
uint8_t buf[64];
memset(buf, 0, 64);
USB_scanf(buf);
 * */

extern FS_t currentFS;

void ShellTask(){
    uint8_t *cmd_buf = (uint8_t *) kernel_alloc(256);
    char *pwd = (char *) kernel_alloc(256);
    memset(cmd_buf, 0, 256);
    USB_color_printf(LIGHT_CYAN, "%s:/$", UserName);
    while(1){
        TaskTickStart(xShell);

        USB_scanf(cmd_buf);
        while (cmd_buf[0] == 0) USB_scanf(cmd_buf);

        execCMD((char*) cmd_buf);
        memset(cmd_buf, 0, 256);

        ram_pwd(currentFS, pwd);

        USB_color_printf(LIGHT_CYAN, "%s:%s$", UserName, pwd);

        // 等待执行串口指令
        osDelay(1000);
        TaskTickEnd(xShell);
    }
}