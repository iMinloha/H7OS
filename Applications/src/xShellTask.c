#include "xShellTask.h"
#include "cmsis_os.h"
#include "RAMFS.h"
#include "TaskHead.h"

extern Task_t xShell;


// ÷’∂Àœﬂ≥Ã—≠ª∑
void ShellTask(){
    osDelay(1000);
    while(1){
        TaskTickStart(xShell);
        execCMD("info /proc/xTaskTest");
//        execCMD("info /proc/xTaskManager");
//        execCMD("info /proc/xShell");
        osDelay(1000);
        TaskTickEnd(xShell);
    }
}