#include "xShellTask.h"
#include "cmsis_os.h"
#include "RAMFS.h"

void taskShellInit(){
    osDelay(1000);
}

void taskLoop(){
    execCMD("cd /mnt");
    execCMD("ls");
    osDelay(1000);
}