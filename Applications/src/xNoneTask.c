/**
 * @file    xNoneTask.c
 * @brief   Idle task — 空闲时闪烁 LED / blinks LED when idle
 *
 * 所有硬件访问通过 Platform API, 不直接调用 HAL。
 * All hardware access through Platform API, no direct HAL calls.
 */

#include <stdio.h>
#include "xNoneTask.h"
#include "cmsis_os.h"
#include "xTaskManager.h"
#include "TaskHead.h"
#include "platform.h"
#include "board_select.h"

extern Task_t xNoneTask;

void NoneTask(void const * argument){
    while(1){
        TaskTickStart(xNoneTask);
#if BOARD_HAS_GPIO
        Platform_GPIO_TogglePin((void *)BSP_LED_PORT, BSP_LED_PIN);
#endif
        osDelay(100);
        TaskTickEnd(xNoneTask);
    }
}
