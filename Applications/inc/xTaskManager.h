#ifndef H7OS_XTASKMANAGER_H
#define H7OS_XTASKMANAGER_H

#include "cmsis_os.h"

enum TaskStatus{
    TASK_READY,
    TASK_RUNNING,
    TASK_SUSPEND,
    TASK_STOP
};

typedef enum TaskStatus TaskStatus_E;

// 任务权限枚举
enum TaskPriority{
    TASK_PRIORITY_NORMAL,
    TASK_PRIORITY_HIGH,
    TASK_PRIORITY_ROOT,
    TASK_PRIORITY_SYSTEM,
};

typedef enum TaskPriority TaskPriority_E;


typedef struct Task* Task_t;

// 任务管理器
struct Task{
    // 任务ID
    char *name;
    // 任务状态
    TaskStatus_E status;
    // CPU占用率
    float cpu;
    // 任务权限
    TaskPriority_E priority;
    // 任务句柄
    osThreadId handle;
    // 计数
    uint32_t lastWakeTime;
    // 累计时间
    uint32_t accumulatedTime;
    // PID
    uint8_t PID;
    // 下一个任务
    Task_t next;
};


void TaskManager(void const * argument);

#endif //H7OS_XTASKMANAGER_H
