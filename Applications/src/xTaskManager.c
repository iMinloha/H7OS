#include <string.h>
#include "xTaskManager.h"
#include "TaskHead.h"
#include "xShellTask.h"
#include "xTaskInit.h"
#include "memctl.h"
#include "RAMFS.h"
#include "test.h"

Task_t xTaskManager;
Task_t xShell;
Task_t xTest;
Task_t xNoneTask;

uint8_t PID_Global = 0;

Task_t RAMFS_TASK_Create(char *name, TaskStatus_E status, TaskPriority_E priority, osThreadId handle){
    Task_t task = (Task_t) kernel_alloc(sizeof(struct Task));
    task->name = kernel_alloc(strlen(name) + 1);
    strcpy(task->name, name);
    task->status = status;
    task->PID = PID_Global++;
    task->cpu = 0;
    task->priority = priority;
    task->handle = handle;
    task->lastWakeTime = 0;
    task->accumulatedTime = 0;
    task->next = NULL;
    return task;
}

void ThreadInit(){
    osThreadDef(xTaskInit, QueueInit, osPriorityNormal, 0, 1024);
    xTaskInitHandle = osThreadCreate(osThread(xTaskInit), NULL);

    osThreadDef(xShell, ShellTask, osPriorityNormal, 0, 1024);
    xShellHandle = osThreadCreate(osThread(xShell), NULL);

    osThreadDef(xTaskManager, TaskManager, osPriorityAboveNormal, 0, 256);
    xTaskManagerHandle = osThreadCreate(osThread(xTaskManager), NULL);

    osThreadDef(xTaskTest, testFunc, osPriorityNormal, 0, 512);
    xTaskTestHandle = osThreadCreate(osThread(xTaskTest), NULL);

    xTaskManager = RAMFS_TASK_Create("xTaskManager", TASK_READY, TASK_PRIORITY_SYSTEM, xTaskManagerHandle);
    xShell      = RAMFS_TASK_Create("xShell",       TASK_READY, TASK_PRIORITY_SYSTEM, xShellHandle);
    xTest       = RAMFS_TASK_Create("xTaskTest",    TASK_READY, TASK_PRIORITY_NORMAL, xTaskTestHandle);
    xNoneTask   = RAMFS_TASK_Create("xNoneTask",    TASK_READY, TASK_PRIORITY_NORMAL, xNoneHandle);
}

extern CPU_t CortexM7;

void TaskManager(void const * argument){
    addThread(xShell);
    addThread(xTaskManager);
    addThread(xTest);
    addThread(xNoneTask);

    Task_t head = getTaskList();
    uint32_t lastTick = xTaskGetTickCount();

    while(1){
        TaskTickStart(xTaskManager);
        uint32_t now = xTaskGetTickCount();
        uint32_t dt = now - lastTick;
        if (dt == 0) dt = 1;
        lastTick = now;

        uint32_t totalAcc = 0;
        Task_t t = head;
        while (t) {
            /* CPU% = task_acc_time / total_time * 100 */
            t->cpu = (float)t->accumulatedTime * 100.0f / (float)dt;
            if (t->cpu > 100.0f) t->cpu = 100.0f;
            totalAcc += t->accumulatedTime;
            t->accumulatedTime = 0;  /* reset for next interval */
            t = t->next;
        }

        CortexM7->load = 100.0f - xNoneTask->cpu;
        if (CortexM7->load < 0) CortexM7->load = 0;

        osDelay(1000);
        TaskTickEnd(xTaskManager);
    }
}
