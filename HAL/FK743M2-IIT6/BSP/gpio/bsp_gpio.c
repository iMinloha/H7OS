#include "bsp_gpio.h"
#include "Core/DrT.h"
#include "memctl.h"
#include <stdio.h>
#include <string.h>

/* ── GPIO fops ───────────────────────────────────────────── */

static int gpio_open(void *dev)  { (void)dev; return 0; }
static int gpio_close(void *dev) { (void)dev; return 0; }

static int gpio_read(void *dev, uint8_t *buf, uint32_t len) {
    gpio_dev_t *g = (gpio_dev_t*)dev;
    if (!g || len < 1) return -1;
    buf[0] = (HAL_GPIO_ReadPin(g->port, g->pin) == GPIO_PIN_SET) ? '1' : '0';
    return 1;
}

static int gpio_write(void *dev, const uint8_t *buf, uint32_t len) {
    gpio_dev_t *g = (gpio_dev_t*)dev;
    if (!g || len < 1 || !g->is_output) return -1;
    HAL_GPIO_WritePin(g->port, g->pin, (buf[0] == '0') ? GPIO_PIN_RESET : GPIO_PIN_SET);
    return (int)len;
}

static bsp_file_ops_t gpio_fops = { gpio_open, gpio_close, gpio_read, gpio_write };

/* ── Registration ────────────────────────────────────────── */

void gpio_register(GPIO_TypeDef *port, uint16_t pin, const char *name, uint8_t is_output) {
    gpio_dev_t *data = (gpio_dev_t*)kernel_alloc(sizeof(gpio_dev_t));
    data->port = port; data->pin = pin; data->is_output = is_output;
    addDevice("dev/gpio", data, (char*)name, "GPIO pin", DEVICE_SERIAL, DEVICE_ON, NULL);
    char _path[64]; sprintf(_path, "/dev/gpio/%s", name);
    DrTNode_t d = loadDevice(_path);
    if (d) d->fops = (void*)&gpio_fops;
}

void gpio_device_init(void)
{
    GPIO_OUTPUT(GPIOF, GPIO_PIN_7, "PF7");
    GPIO_OUTPUT(GPIOH, GPIO_PIN_7, "PH7");
}
