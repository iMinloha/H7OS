#ifndef H7OS_CPU_H
#define H7OS_CPU_H

#include <stdint-gcc.h>
#include "RAMFS.h"
#include "cmsis_os.h"

typedef float (*CPU_Func_t)(void);

struct CPU{
    // CPU名称
    char* name;
    // CPU描述
    char* description;
    // CPU频率
    uint32_t frequency;
    // CPU温度
    double temperature;
    // CPU负载
    double load;
};
// CPU对象
typedef struct CPU* CPU_t;

/**
 * 创建CPU对象
 * @return CPU对象
 * @note 该函数会自动初始化CPU对象，对象为CortexM7
 */
void createCPU();

void showCPUInfo();


#endif //H7OS_CPU_H
