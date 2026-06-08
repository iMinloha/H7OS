/**
 * @file    bsp_devices.h
 * @brief   Platform 内部接口 — BSP 设备注册与板级外设初始化
 *          Platform internal interface — BSP device registration & board peripheral init
 *
 * 本文件仅供 Platform 层和 HAL/BSP 层内部使用。
 * Software 层请使用 platform.h 中的 Platform_DevicesInit() 和 Platform_Init()。
 *
 * This file is for Platform and HAL/BSP internal use only.
 * Software layer should use Platform_DevicesInit() and Platform_Init() from platform.h.
 */

#ifndef BSP_DEVICES_H
#define BSP_DEVICES_H

#include <stdint.h>

/**
 * @brief  将板载外设注册到 DrT 设备树 (cpu, usart, gpio, pwm, adc, i2c, ...)
 *         Register onboard peripherals to DrT device tree
 * @note   由 HAL/<板名>/BSP/bsp_devices.c 实现 / Implemented by HAL/<board>/BSP/bsp_devices.c
 *         Software 层应通过 Platform_DevicesInit() 间接调用
 */
void devices_init(void);

/**
 * @brief  板级外设硬件初始化 (SD卡检测, 触摸屏, USB CDC, DFU检测)
 *         Board-level peripheral hardware init (SD detect, Touch, USB CDC, DFU check)
 * @note   由 HAL/<板名>/BSP/bsp_devices.c 实现
 *         Software 层应通过 Platform_Init() 间接调用
 */
void board_periph_init(void);

/**
 * @brief  获取 SD 卡容量 (字节) / Get SD card capacity (bytes)
 * @return 容量 / capacity, 0 = 无卡或错误 / no card or error
 */
uint64_t board_sd_get_capacity(void);

/**
 * @brief  设置 RTC DFU 标记 / Set RTC DFU flag
 */
void board_rtc_set_dfu_flag(void);

#endif /* BSP_DEVICES_H */
