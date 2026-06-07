#include "bsp_devices.h"

/* Include all device drivers */
#include "Components/cpu/bsp_cpu.h"
#include "Components/usart/bsp_usart.h"
#include "Components/gpio/bsp_gpio.h"
#include "Components/pwm/bsp_pwm.h"
#include "Components/i2c/bsp_i2c.h"
#include "Components/adc/bsp_adc.h"

void devices_init(void) {
    cpu_device_init();
    usart1_device_init();
    gpio_device_init();
    pwm_device_init();
    i2c_device_init();
    adc_device_init();
}
