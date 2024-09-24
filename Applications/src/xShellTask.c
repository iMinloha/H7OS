#include <stdio.h>
#include "xShellTask.h"
#include "cmsis_os.h"
#include "RAMFS.h"
#include "TaskHead.h"
#include "torch_iic.h"

extern Task_t xShell;

extern volatile TouchStructure touchInfo;


void ShellTask(){

    osDelay(1000);
    while(1){
        TaskTickStart(xShell);
//        lv_task_handler();
//        Touch_Scan();
//        printf("Touch: %d %d\n", touchInfo.x[0], touchInfo.y[0]);
//        execCMD("info /proc/xTaskTest");
//        execCMD("info /proc/xTaskInit");

//        execCMD("info /proc/xTaskManager");
//        execCMD("info /proc/xShell");
        osDelay(1000);
        TaskTickEnd(xShell);
    }
}