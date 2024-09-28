#include <stdio.h>
#include "xShellTask.h"
#include "cmsis_os.h"
#include "RAMFS.h"
#include "TaskHead.h"
#include "torch_iic.h"
#include "ltdc.h"
#include "usbd_cdc_if.h"
#include "memctl.h"
#include "quadspi.h"

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
    osDelay(1000);
    uint8_t *cmd_buf = (uint8_t *) kernel_alloc(256);
    char *pwd = (char *) kernel_alloc(256);
    memset(cmd_buf, 0, 256);
    USB_printf("%s@$/:", UserName);
    while(1){
        TaskTickStart(xShell);

        USB_scanf(cmd_buf);
        while (cmd_buf[0] == 0) USB_scanf(cmd_buf);

        execCMD((char*) cmd_buf);
        memset(cmd_buf, 0, 256);

        ram_pwd(currentFS, pwd);
        USB_printf("%s@$%s:", UserName, pwd);

        // 等待执行串口指令
        osDelay(1000);
        TaskTickEnd(xShell);
    }
}