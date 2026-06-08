#include "bsp_i2c.h"
#include "Core/DrT.h"
#include "memctl.h"
#include <stdio.h>

static int i2c_write(void *dev, const uint8_t *buf, uint32_t len) {
    I2C_HandleTypeDef *h = (I2C_HandleTypeDef*)dev;
    if (len < 2) return -1;
    uint16_t addr = buf[0] << 1;
    if (HAL_I2C_Master_Transmit(h, addr, (uint8_t*)buf+1, len-1, 100) == HAL_OK)
        return (int)len;
    return -1;
}

static int i2c_read(void *dev, uint8_t *buf, uint32_t len) {
    I2C_HandleTypeDef *h = (I2C_HandleTypeDef*)dev;
    if (len < 2) return -1;
    uint16_t addr = buf[0] << 1;
    if (HAL_I2C_Master_Receive(h, addr, buf+1, len-1, 100) == HAL_OK)
        return (int)len;
    return -1;
}

static bsp_file_ops_t i2c_fops = { NULL, NULL, i2c_read, i2c_write };

void i2c_register(I2C_HandleTypeDef *hi2c, const char *name) {
    addDevice("dev/i2c", hi2c, (char*)name, "I2C bus", DEVICE_SERIAL, DEVICE_ON, NULL);
    char _path[64]; sprintf(_path, "/dev/i2c/%s", name);
    DrTNode_t d = loadDevice(_path);
    if (d) d->fops = (void*)&i2c_fops;
}

void i2c_device_init(void) {
    I2C_REGISTER(hi2c1, "I2C1");
}
