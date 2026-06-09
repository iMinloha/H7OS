#include "bsp_pwm.h"
#include "dev_register.h"
#include "memctl.h"
#include <stdio.h>

typedef struct { TIM_HandleTypeDef *htim; uint32_t channel; } pwm_dev_t;

static int pwm_write(void *dev, const uint8_t *buf, uint32_t len) {
    pwm_dev_t *p = (pwm_dev_t*)dev;
    if (!p || len < 1) return -1;
    uint32_t duty = 0;
    for (uint32_t i = 0; i < len && buf[i] >= '0' && buf[i] <= '9'; i++)
        duty = duty * 10 + (buf[i] - '0');
    if (duty > 100) duty = 100;
    uint32_t period = __HAL_TIM_GET_AUTORELOAD(p->htim);
    uint32_t pulse = (duty * (period + 1)) / 100;
    __HAL_TIM_SET_COMPARE(p->htim, p->channel, pulse);
    HAL_TIM_PWM_Start(p->htim, p->channel);
    return (int)len;
}

static int pwm_read(void *dev, uint8_t *buf, uint32_t len) {
    (void)dev; if (len > 0) { buf[0] = '0'; return 1; } return 0;
}

static bsp_file_ops_t pwm_fops = { NULL, NULL, pwm_read, pwm_write };

void pwm_register(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t freq_hz, const char *name) {
    /* TIM2 on APB1: clock = PCLK1 * 2 = 240 MHz (since APB1 divider = 2) */
    uint32_t tim_clk = HAL_RCC_GetPCLK1Freq() * 2;
    htim->Init.Prescaler = 0;
    htim->Init.Period = (tim_clk / freq_hz) - 1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_PWM_Init(htim);

    pwm_dev_t *data = (pwm_dev_t*)kernel_alloc(sizeof(pwm_dev_t));
    data->htim = htim; data->channel = channel;
    dev_register("dev/pwm", data, name, "PWM output", DEVICE_TIMER, &pwm_fops);
}

void pwm_device_init(void) {
    PWM_REGISTER(htim2, TIM_CHANNEL_1, 50, "PWM2_CH1");
    PWM_REGISTER(htim2, TIM_CHANNEL_2, 50, "PWM2_CH2");
}
