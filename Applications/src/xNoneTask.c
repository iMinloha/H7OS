#include <stdio.h>
#include "xNoneTask.h"
#include "cmsis_os.h"
#include "xTaskManager.h"
#include "TaskHead.h"
#include "gpio.h"

extern Task_t xNoneTask;


void NoneTask(void const * argument){
    while(1){
        TaskTickStart(xNoneTask);
        HAL_GPIO_TogglePin(GPIOH, GPIO_PIN_7);
        osDelay(100);
        TaskTickEnd(xNoneTask);
    }
}