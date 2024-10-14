#ifndef H7OS_TASKHEAD_H
#define H7OS_TASKHEAD_H

#include "cmsis_os.h"
#include "xTaskManager.h"

#define UserName "H7OS"

// 所有线程都需要放在这里进行注册
static osThreadId xTaskInitHandle; // 在FreeRTOS初始化后进行的初始化，然后就死了
static osThreadId xTaskManagerHandle; // 任务管理器，用于统计线程占用的
static osThreadId xShellHandle; // shell线程
static osThreadId xTaskTestHandle;  // 测试线程
static osThreadId xNoneHandle; // 咸鱼线程，用于测量CPU空闲度的

#define TaskTickStart(task) task->lastWakeTime = xTaskGetTickCount();
#define TaskTickEnd(task) task->accumulatedTime = xTaskGetTickCount() - task->lastWakeTime;

void ThreadInit();

#endif //H7OS_TASKHEAD_H
