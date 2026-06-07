#ifndef H7OS_CPU_H
#define H7OS_CPU_H

#include <stdint-gcc.h>
#include "cmsis_os.h"

struct CPU {
    char    *name;
    char    *description;
    uint32_t frequency;
    double   temperature;
    double   load;
};

typedef struct CPU* CPU_t;

extern CPU_t CortexM7;

void  createCPU(void);
void  showCPUInfo(void);
float updateCPU(void);

#endif
