# 添加新设备驱动 / Adding a New Device Driver

本文说明如何为 H7OS 添加一个新的外设驱动 (如 SPI, CAN, DAC...)。

## 概览 / Overview

```
1. 在 HAL/<board>/BSP/ 下创建驱动目录
2. 编写 bsp_xxx.h (设备结构体 + 注册宏)
3. 编写 bsp_xxx.c (file_ops 实现)
4. 在 bsp_devices.c 注册
5. 在 board_xxx.h 添加 BOARD_HAS_XXX 开关
6. (可选) 添加对应的 use 子命令
```

## 详细步骤 / Step by Step

### 1. 创建驱动文件 / Create Driver Files

```bash
mkdir HAL/MyBoard/BSP/spi
touch HAL/MyBoard/BSP/spi/bsp_spi.h
touch HAL/MyBoard/BSP/spi/bsp_spi.c
```

### 2. 编写头文件 / Write Header

```c
// HAL/MyBoard/BSP/spi/bsp_spi.h
#ifndef BSP_SPI_H
#define BSP_SPI_H

#include "bsp_file_ops.h"
#include "stm32h7xx_hal.h"  // for SPI_HandleTypeDef

/* 设备数据结构 / device data */
typedef struct {
    SPI_HandleTypeDef *hspi;
    uint32_t           mode;       // 0=master, 1=slave
} spi_dev_t;

/* 注册函数 / register function */
void spi_register(SPI_HandleTypeDef *hspi, uint32_t mode, const char *name);

/* 注册宏 / registration macro */
#define SPI_REGISTER(hspi, mode, name) \
    spi_register(&hspi, mode, name)

/* device_init 入口 / device init entry */
void spi_device_init(void);

#endif
```

### 3. 编写源文件 / Write Source

```c
// HAL/MyBoard/BSP/spi/bsp_spi.c
#include "bsp_spi.h"
#include "Core/DrT.h"
#include "memctl.h"

/* ── file_ops 实现 / file_ops implementation ────────────── */

static int spi_open(void *dev)  { (void)dev; return 0; }
static int spi_close(void *dev) { (void)dev; return 0; }

static int spi_write(void *dev, const uint8_t *buf, uint32_t len)
{
    spi_dev_t *s = (spi_dev_t *)dev;
    if (!s || len == 0) return -1;
    if (HAL_SPI_Transmit(s->hspi, (uint8_t *)buf, len, 100) == HAL_OK)
        return (int)len;
    return -1;
}

static int spi_read(void *dev, uint8_t *buf, uint32_t len)
{
    spi_dev_t *s = (spi_dev_t *)dev;
    if (!s || len == 0) return -1;
    if (HAL_SPI_Receive(s->hspi, buf, len, 100) == HAL_OK)
        return (int)len;
    return -1;
}

static bsp_file_ops_t spi_fops = {
    .open  = spi_open,
    .close = spi_close,
    .read  = spi_read,
    .write = spi_write,
};

/* ── 注册 / registration ────────────────────────────────── */

void spi_register(SPI_HandleTypeDef *hspi, uint32_t mode, const char *name)
{
    spi_dev_t *data = (spi_dev_t *)kernel_alloc(sizeof(spi_dev_t));
    data->hspi = hspi;
    data->mode = mode;

    addDevice("dev/spi", data, (char *)name,
        "SPI device", DEVICE_SERIAL, DEVICE_ON, NULL);

    char _path[64];
    sprintf(_path, "/dev/spi/%s", name);
    DrTNode_t d = loadDevice(_path);
    if (d) d->fops = (void *)&spi_fops;
}

/* ── device_init / 初始化入口 ────────────────────────────── */

void spi_device_init(void)
{
    SPI_REGISTER(hspi1, 0, "SPI1");
    // SPI_REGISTER(hspi2, 0, "SPI2");  // 按需添加 / add as needed
}
```

### 4. 注册到系统 / Register in System

在 `HAL/MyBoard/BSP/bsp_devices.c` 的 `devices_init()` 中添加:

```c
#if BOARD_HAS_SPI
    #include "spi/bsp_spi.h"
#endif

void devices_init(void) {
    // ... existing ...
#if BOARD_HAS_SPI
    spi_device_init();
#endif
}
```

### 5. 添加外设开关 / Add Peripheral Switch

在 `Platform/Board/board_myboard.h` 中添加:

```c
#define BOARD_HAS_SPI       1   // 启用 SPI / enable SPI
```

在 `Platform/Board/board_select.h` 的默认值区添加:

```c
#ifndef BOARD_HAS_SPI
    #define BOARD_HAS_SPI   0
#endif
```

### 6. 创建设备树目录 / Create Device Tree Dir

在 `RAMFS/init.d/Core/DrT.c` 的 `DrTInit()` 中添加:

```c
fs_create_child(dev, "spi", 0);  // 创建 /dev/spi/ 目录
```

### 7. (可选) 添加 Shell 命令 / Add Shell Command

```c
// Command/spi/spi_main.c
#include "Core/DrT.h"
#include "usbd_cdc_if.h"

void spi_main(int argc, char **argv)
{
    if (argc < 2) {
        USB_printf("Usage: spi <device> <send|recv> [data]\n");
        return;
    }
    if (strcmp(argv[1], "send") == 0 && argc >= 3) {
        dev_write(argv[0], (uint8_t *)argv[2], strlen(argv[2]));
        USB_printf("spi: sent %d bytes\n", (int)strlen(argv[2]));
    } else if (strcmp(argv[1], "recv") == 0) {
        uint8_t buf[64];
        int n = dev_read(argv[0], buf, sizeof(buf) - 1);
        if (n > 0) { buf[n] = '\0'; USB_printf("spi: %s\n", buf); }
    }
}
```

## 驱动模式参考 / Driver Pattern Reference

所有 BSP 驱动遵循以下模式:

```c
// 1. 设备数据结构 / device data struct
typedef struct { HAL_Handle *h; uint32_t config; } xxx_dev_t;

// 2. file_ops / 文件操作
static bsp_file_ops_t xxx_fops = { xxx_open, xxx_close, xxx_read, xxx_write };

// 3. 注册函数 / register
void xxx_register(HAL_Handle *h, const char *name) {
    xxx_dev_t *d = kernel_alloc(sizeof(xxx_dev_t));
    d->h = h;
    addDevice("dev/xxx", d, name, "desc", TYPE, DEVICE_ON, NULL);
    DrTNode_t n = loadDevice("/dev/xxx/name");
    if (n) n->fops = (void*)&xxx_fops;
}

// 4. device_init / 初始化入口
void xxx_device_init(void) { XXX_REGISTER(hxxx1, "XXX1"); }

// 5. 注册宏 / registration macro
#define XXX_REGISTER(handle, name) xxx_register(&handle, name)
```

## 已注册的设备类型 / Registered Device Types

| 类型 / Type | 枚举 / Enum | 注册宏示例 |
|---|---|---|
| GPIO | DEVICE_SERIAL | `GPIO_OUTPUT(GPIOH, GPIO_PIN_7, "PH7")` |
| USART | DEVICE_SERIAL | `usart1_device_init()` |
| PWM | DEVICE_TIMER | `PWM_REGISTER(htim2, TIM_CHANNEL_1, 50, "PWM2_CH1")` |
| ADC | DEVICE_VOTAGE | `ADC_REGISTER(hadc1, "ADC1")` |
| I2C | DEVICE_SERIAL | `I2C_REGISTER(hi2c1, "I2C1")` |
| CPU | DEVICE_BS | `cpu_device_init()` |
| SPI (新) | DEVICE_SERIAL | `SPI_REGISTER(hspi1, 0, "SPI1")` |
