#ifndef H7OS_XTASKINIT_H
#define H7OS_XTASKINIT_H


#include "cmsis_os.h"

static osThreadId xTaskInitHandle;

// 在FreeRTOS初始化前进行的初始化
void taskGlobalInit();

// 在FreeRTOS初始化后进行的初始化
void QueueInit(void const * argument);

#endif
