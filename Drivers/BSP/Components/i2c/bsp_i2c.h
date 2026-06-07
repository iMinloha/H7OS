#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "bsp_file_ops.h"
#include "i2c.h"

void i2c_register(I2C_HandleTypeDef *hi2c, const char *name);
#define I2C_REGISTER(hi2c, name) i2c_register(&hi2c, name)
void i2c_device_init(void);

#endif
