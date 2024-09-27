#include "cpu.h"
#include "memctl.h"
#include "adc.h"
#include "stdio.h"
#include "usbd_cdc_if.h"

CPU_t CortexM7;

float updateCPU(){
    HAL_ADC_Start(&hadc3);
    uint16_t adc_v = HAL_ADC_GetValue(&hadc3);
    double adc_x = (110.0-30.0)/(*(unsigned short*)(0x1FF1E840) - *(unsigned short*)(0x1FF1E820));
    return adc_x * (adc_v - *(unsigned short*)(0x1FF1E820)) + 30;
}

/**
 * 创建CPU对象
 * @return CPU对象
 * @note 该函数会自动初始化CPU对象，对象为CortexM7
 */
void createCPU(){
    CortexM7 = (CPU_t) kernel_alloc(sizeof(struct CPU));
    CortexM7->name = "Cortex-M7";
    CortexM7->description = "Cortex-M7 CPU";
    CortexM7->frequency = HAL_RCC_GetSysClockFreq();
    CortexM7->temperature = 0;
}

void showCPUInfo(){
    USB_printf("CPU name: %s\n", CortexM7->name);
    USB_printf("CPU description: %s\n", CortexM7->description);
    USB_printf("CPU frequency: %lu HZ\n", CortexM7->frequency);
    float temp = updateCPU();
    USB_printf("CPU temperature: %f C\n", temp);
    USB_printf("CPU load: %f%%\n", CortexM7->load);
    USB_printf("====================================\n");
}