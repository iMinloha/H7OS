#ifndef H7OS_TASKHEAD_H
#define H7OS_TASKHEAD_H

#include "cmsis_os.h"
#include "xTaskManager.h"

#define UserName "root"

/* RTOS thread handles (created in ThreadInit) */
static osThreadId xTaskInitHandle;
static osThreadId xTaskManagerHandle;
static osThreadId xShellHandle;
static osThreadId xTaskTestHandle;
static osThreadId xNoneHandle;

#define TaskTickStart(task) (task)->lastWakeTime = xTaskGetTickCount()
#define TaskTickEnd(task)   (task)->accumulatedTime += xTaskGetTickCount() - (task)->lastWakeTime

void ThreadInit(void);

#endif
