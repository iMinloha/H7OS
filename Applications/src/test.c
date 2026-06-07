#include "test.h"
#include "cmsis_os.h"
#include "TaskHead.h"

extern Task_t xTest;

void testFunc(){
    while(1){
        TaskTickStart(xTest);
        osDelay(1000);
        TaskTickEnd(xTest);
    }
}
