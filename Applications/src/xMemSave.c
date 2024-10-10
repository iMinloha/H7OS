#include "xMemSave.h"
#include "xTaskManager.h"
#include "TaskHead.h"
#include "init.d/CPU/CS.h"

extern Task_t xMemSave;

#define Second(x) x * 1000

void MemSaveTask(void const * argument){
    while(1){
        TaskTickStart(xMemSave);
        // 定时保存指令
        // CS_save();
        osDelay(Second(60));
        TaskTickEnd(xMemSave);
    }
}