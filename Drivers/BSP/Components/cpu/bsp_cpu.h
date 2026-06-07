#ifndef BSP_CPU_H
#define BSP_CPU_H

#include "bsp_file_ops.h"

extern bsp_file_ops_t cpu_fops;

/** Call from device_init() to register CPU under /dev */
void cpu_device_init(void);

#endif
