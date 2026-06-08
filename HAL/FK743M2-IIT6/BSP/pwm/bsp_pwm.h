#ifndef BSP_PWM_H
#define BSP_PWM_H

#include "bsp_file_ops.h"
#include "tim.h"

void pwm_register(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t freq_hz, const char *name);
#define PWM_REGISTER(htim, ch, freq, name) pwm_register(&htim, ch, freq, name)
void pwm_device_init(void);

#endif
