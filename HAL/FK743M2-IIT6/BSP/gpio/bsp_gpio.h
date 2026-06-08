#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "bsp_file_ops.h"
#include "stm32h7xx_hal.h"

/** Per-pin GPIO device data (stored in DrTNode.data) */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint8_t       is_output;  /* 1=output, 0=input */
} gpio_dev_t;

/** Register a GPIO pin as a device under /dev */
void gpio_register(GPIO_TypeDef *port, uint16_t pin,
                   const char *name, uint8_t is_output);

#define GPIO_OUTPUT(port, pin, name) \
    gpio_register(port, pin, name, 1)
#define GPIO_INPUT(port, pin, name)  \
    gpio_register(port, pin, name, 0)

/** Called from devices_init() */
void gpio_device_init(void);

#endif
