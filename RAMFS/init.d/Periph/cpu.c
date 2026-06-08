/**
 * @file    cpu.c
 * @brief   CPU 信息对象 — 温度/频率/负载
 *          CPU info object — temperature/frequency/load
 *
 * 所有硬件访问通过 Platform API。
 * All hardware access through Platform API.
 */

#include "cpu.h"
#include "memctl.h"
#include "stdio.h"
#include "usbd_cdc_if.h"
#include "platform.h"

CPU_t CortexM7;

float updateCPU() {
    return Platform_CPU_GetTemperature();
}

/**
 * @brief  创建 CPU 对象 (在 /dev/cpu 注册之前调用)
 *         Create CPU object (called before /dev/cpu registration)
 * @note   对象分配在 kernel TLSF 池中 / Object allocated in kernel TLSF pool
 */
void createCPU() {
    if (CortexM7) return;
    CortexM7 = (CPU_t)kernel_alloc(sizeof(struct CPU));
    CortexM7->name = "CPU";
    CortexM7->description = "STM32 Cortex-M7";
    CortexM7->frequency = Platform_GetSysClockFreq();
    CortexM7->temperature = 0;
    CortexM7->load = 0;
}

void showCPUInfo() {
    USB_printf("CPU name: %s\n", CortexM7->name);
    USB_printf("CPU description: %s\n", CortexM7->description);
    USB_printf("CPU frequency: %lu HZ\n", (unsigned long)CortexM7->frequency);
    float temp = updateCPU();
    USB_printf("CPU temperature: %.1f C\n", temp);
    USB_printf("CPU load: %.1f%%\n", CortexM7->load);
    USB_printf("====================================\n");
}
