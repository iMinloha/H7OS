#ifndef BSP_ADC_H
#define BSP_ADC_H

#include "bsp_file_ops.h"
#include "adc.h"

void adc_register(ADC_HandleTypeDef *hadc, const char *name);
#define ADC_REGISTER(hadc, name) adc_register(&hadc, name)
void adc_device_init(void);

#endif
