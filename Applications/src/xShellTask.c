#include "xShellTask.h"
#include "cmsis_os.h"
#include "RAMFS.h"
#include "u_stdio.h"
#include "adc.h"

// 终端线程初始化，延时为了等待其他线程初始化完成
void taskShellInit(){
    osDelay(1000);
}

// 终端线程循环
void taskLoop(){
    // execCMD("help ls");
    osDelay(1000);
}