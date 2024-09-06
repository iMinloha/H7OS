#include "xTaskManager.h"
#include "TaskHead.h"
#include "xShellTask.h"
#include "xTaskInit.h"
#include "memctl.h"
#include "u_stdio.h"
#include "RAMFS.h"
#include "test.h"


Task_t xTaskManager;
Task_t xShell;
Task_t xTest;

uint8_t PID_Global = 0;

Task_t RAMFS_TASK_Create(char *name, TaskStatus_E status, TaskPriority_E priority, osThreadId handle){
    Task_t task = (Task_t) kernel_alloc(sizeof(struct Task));

    task->name = kernel_alloc(strlen(name) + 1);
    strcopy(task->name, name);

    task->status = status;
    task->PID = PID_Global++;
    task->cpu = 0;
    task->priority = priority;
    task->handle = handle;
    task->next = NULL;
    return task;
}


void ThreadInit(){
    // 线程都在这里进行注册
    osThreadDef(xShell, ShellTask, osPriorityNormal, 0, 1024);
    xShellHandle = osThreadCreate(osThread(xShell), NULL);

    osThreadDef(xTaskManager, TaskManager, osPriorityAboveNormal, 0, 1024);
    xTaskManagerHandle = osThreadCreate(osThread(xTaskManager), NULL);

    osThreadDef(xTaskInit, QueueInit, osPriorityIdle, 0, 2048);
    xTaskInitHandle = osThreadCreate(osThread(xTaskInit), NULL);

    osThreadDef(xTaskTest, testFunc, osPriorityRealtime, 0, 2048);
    xTaskTestHandle = osThreadCreate(osThread(xTaskTest), NULL);

    xTaskManager = RAMFS_TASK_Create("xTaskManager", TASK_READY, TASK_PRIORITY_SYSTEM, xTaskManagerHandle);
    xShell = RAMFS_TASK_Create("xShell", TASK_READY, TASK_PRIORITY_SYSTEM, xShellHandle);
    xTest = RAMFS_TASK_Create("xTaskTest", TASK_READY, TASK_PRIORITY_NORMAL, xTaskTestHandle);
}

void TaskManager(void const * argument){
    addThread(xShell);
    addThread(xTaskManager);
    addThread(xTest);

    Task_t head = getTaskList();

    uint32_t lastTotalTicks = xTaskGetTickCount();

    while(1){
        TaskTickStart(xTaskManager);
        Task_t currentTask = head;

        uint32_t currentTotalTicks = xTaskGetTickCount(); // 当前系统总滴答数
        uint32_t deltaTime = currentTotalTicks - lastTotalTicks; // 计算自上次统计以来的增量时间


        while (currentTask != NULL){

            currentTask->cpu = (float)currentTask->accumulatedTime / (10 * deltaTime) * 100.0f;

            currentTask = currentTask->next;
        }
        lastTotalTicks = currentTotalTicks;

        osDelay(1000);
        TaskTickEnd(xTaskManager);
    }
}