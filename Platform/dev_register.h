#ifndef DEV_REGISTER_H
#define DEV_REGISTER_H

#include <stdio.h>
#include "Core/DrT.h"

/**
 * @brief 统一设备注册 — 替代 BSP 中手写 addDevice + loadDevice + fops
 *
 * 用法 (在 BSP init 函数中):
 *   gpio_dev_t *d = kernel_alloc(sizeof(gpio_dev_t));
 *   d->port = GPIOH; d->pin = GPIO_PIN_7; d->is_output = 1;
 *   dev_register("dev/gpio", d, "PH7", "GPIO pin", DEVICE_SERIAL, &gpio_fops);
 *
 * @param dir     设备目录 (不含前导 "/"), 如 "dev/gpio"
 * @param data    BSP 私有设备数据结构 (kernel_alloc 分配的)
 * @param name    设备名称
 * @param desc    设备描述
 * @param type    设备类型 (DEVICE_SERIAL, DEVICE_VOTAGE, ...)
 * @param fops    bsp_file_ops_t* 文件操作表
 * @return 0=成功, -1=失败
 */
static inline int dev_register(const char *dir, void *data, const char *name,
                                const char *desc, DeviceType_E type, void *fops) {
    addDevice((char*)dir, data, (char*)name, (char*)desc, type, DEVICE_ON, NULL);
    char path[64];
    snprintf(path, sizeof(path), "/%s/%s", dir, name);
    DrTNode_t d = loadDevice(path);
    if (d) { d->fops = fops; return 0; }
    return -1;
}

#endif
