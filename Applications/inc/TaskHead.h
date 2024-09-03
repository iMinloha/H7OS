#ifndef H7OS_TASKHEAD_H
#define H7OS_TASKHEAD_H

#include "cmsis_os.h"

// 所有线程都需要放在这里进行注册
static osThreadId xTaskInitHandle;
static osThreadId xTaskManagerHandle;
static osThreadId xShellHandle;

void ThreadInit();

#endif //H7OS_TASKHEAD_H
