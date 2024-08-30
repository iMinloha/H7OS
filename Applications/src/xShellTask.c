#include "xShellTask.h"
#include "cmsis_os.h"
#include "RAMFS.h"
#include "memctl.h"
#include "u_stdio.h"


void taskShellInit(){
    osDelay(1000);
}

void taskLoop(){
    execCMD("ls /dev");
    osDelay(1000);
}