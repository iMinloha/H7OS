#include "bsp_cpu.h"
#include "Core/DrT.h"
#include "Periph/cpu.h"
#include "memctl.h"
#include "platform.h"
#include <stdio.h>

extern CPU_t CortexM7;

static int cpu_open(void *dev) {
    (void)dev;
    return 0;
}

static int cpu_close(void *dev) {
    (void)dev;
    return 0;
}

static int cpu_read(void *dev, uint8_t *buf, uint32_t len) {
    (void)dev;
    CortexM7->temperature = updateCPU();
    int n = snprintf((char*)buf, len,
        "name: %s\n"
        "freq: %lu Hz\n"
        "temp: %.1f C\n"
        "load: %.1f %%\n",
        CortexM7->name,
        CortexM7->frequency,
        CortexM7->temperature,
        CortexM7->load);
    return n;
}

static int cpu_write(void *dev, const uint8_t *buf, uint32_t len) {
    (void)dev; (void)buf; (void)len;
    return -1;
}

bsp_file_ops_t cpu_fops = {
    .open  = cpu_open,
    .close = cpu_close,
    .read  = cpu_read,
    .write = cpu_write,
};

void cpu_device_init(void)
{
    /* 通过 Platform API 校准并启动内部温度传感器 ADC3 */
    /* Calibrate and start internal temperature sensor ADC3 via Platform API */
    Platform_CPU_TempInit();

    createCPU();
    addDevice("dev/cpu", CortexM7, "CPU",
        "Central Processing Unit", DEVICE_BS, DEVICE_ON, NULL);

    char _path[64];
    sprintf(_path, "/dev/cpu/CPU");
    DrTNode_t d = loadDevice(_path);
    if (d) d->fops = (void *) &cpu_fops;
}
