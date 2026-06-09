/**
 * @file    xNoneTask.c
 * @brief   Idle task — 通过 fops 操作 LED / blinks LED via fops
 *
 * 所有设备 I/O 必须通过 open → write/read → close 路径,
 * 统一走软件层 dev_* 函数, 遵守设备所有权机制.
 */

#include <stdio.h>
#include "xNoneTask.h"
#include "cmsis_os.h"
#include "xTaskManager.h"
#include "TaskHead.h"
#include "Core/DrT.h"

extern Task_t xNoneTask;

// LED设备所在的GPIO
#define LED_DEV   "/dev/gpio/PH7"

void NoneTask(void const * argument){
    /* 打开 LED 设备 */
    if (dev_open(LED_DEV) != 0) {
        /* LED 不可用, 静默退出 */
        osThreadTerminate(NULL);
        return;
    }

    uint8_t val = '0';
    while(1){
        TaskTickStart(xNoneTask);

        val = (val == '0') ? '1' : '0';
        dev_write(LED_DEV, &val, 1);

        osDelay(100);
        TaskTickEnd(xNoneTask);
    }
}
