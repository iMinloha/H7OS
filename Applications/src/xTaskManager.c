#include <string.h>
#include "xTaskManager.h"
#include "TaskHead.h"
#include "xShellTask.h"
#include "xTaskInit.h"
#include "memctl.h"
#include "RAMFS.h"
#include "test.h"
#include "task.h"

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

    xTaskManager = RAMFS_TASK_Create("TaskMgr", TASK_READY, TASK_PRIORITY_SYSTEM, xTaskManagerHandle);
    xShell      = RAMFS_TASK_Create("Shell",    TASK_READY, TASK_PRIORITY_SYSTEM, xShellHandle);
    xTest       = RAMFS_TASK_Create("Test",     TASK_READY, TASK_PRIORITY_NORMAL, xTaskTestHandle);
    xNoneTask   = RAMFS_TASK_Create("Kernel",   TASK_READY, TASK_PRIORITY_NORMAL, xNoneHandle);
}

extern CPU_t CortexM7;

void TaskManager(void const * argument){
    addThread(xShell);
    addThread(xTaskManager);
    addThread(xTest);
    addThread(xNoneTask);

    while(1){
        /* Query FreeRTOS for per-task runtime stats.
         * FreeRTOS automatically tracks actual CPU execution time
         * (not including blocked/sleeping time) via the DWT cycle counter. */
        UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
        TaskStatus_t *pxTaskStatusArray =
            (TaskStatus_t *)kernel_alloc(uxArraySize * sizeof(TaskStatus_t));

        if (pxTaskStatusArray != NULL) {
            uint32_t ulTotalRunTime;
            uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize,
                                               &ulTotalRunTime);

            uint32_t ulIdleRunTime = 0;

            for (UBaseType_t x = 0; x < uxArraySize; x++) {
                /* Match FreeRTOS task to our Task_t by handle */
                Task_t t = getTaskByHandle(pxTaskStatusArray[x].xHandle);
                if (t != NULL) {
                    if (ulTotalRunTime > 0) {
                        t->cpu = (float)pxTaskStatusArray[x].ulRunTimeCounter
                               * 100.0f / (float)ulTotalRunTime;
                        if (t->cpu > 100.0f) t->cpu = 100.0f;
                    } else {
                        t->cpu = 0.0f;
                    }

                    /* Sync task status from FreeRTOS */
                    switch (pxTaskStatusArray[x].eCurrentState) {
                        case eRunning:   t->status = TASK_RUNNING;  break;
                        case eReady:     t->status = TASK_READY;    break;
                        case eBlocked:
                        case eSuspended: t->status = TASK_SUSPEND;  break;
                        case eDeleted:   t->status = TASK_STOP;     break;
                        default: break;
                    }
                }

                /* Track FreeRTOS idle task for system load calculation */
                if (strcmp(pxTaskStatusArray[x].pcTaskName, "IDLE") == 0) {
                    ulIdleRunTime = pxTaskStatusArray[x].ulRunTimeCounter;
                }
            }

            /* System load = 100% - idle CPU percentage */
            if (ulTotalRunTime > 0) {
                CortexM7->load = 100.0f
                    - (float)ulIdleRunTime * 100.0f / (float)ulTotalRunTime;
                if (CortexM7->load < 0.0f) CortexM7->load = 0.0f;
                if (CortexM7->load > 100.0f) CortexM7->load = 100.0f;
            }

            kernel_free(pxTaskStatusArray);
        }

        osDelay(1000);
    }
}
