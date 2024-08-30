#ifndef H7OS_XTASKINIT_H
#define H7OS_XTASKINIT_H


#include "cmsis_os.h"

static osThreadId xTaskInitHandle;

void taskGlobalInit();

void QueueInit(void const * argument);

#endif
