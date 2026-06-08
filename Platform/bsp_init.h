/**
 * @file    bsp_init.h
 * @brief   Platform 层统一初始化接口
 *
 * Platform 层负责:
 *   1. 注册所有板载外设到 DrT 设备树 (通过 devices_init())
 *   2. 板级初始化 (SD卡检测、触摸屏、USB设备等)
 *   3. FreeRTOS 时基配置
 *
 * 用法: 在 main() 中 HAL_BoardInit() 之后调用 Platform_Init()
 */

#ifndef PLATFORM_BSP_INIT_H
#define PLATFORM_BSP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Platform 层统一初始化
 * @note   调用顺序:
 *           1. devices_init() —— 将板载外设注册到 DrT 设备树
 *           2. 板级外设初始化 (SD、Touch、USB)
 *         本函数在 HAL_BoardInit() 之后、FreeRTOS 启动之前调用
 */
void Platform_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_BSP_INIT_H */
