/**
 * @file    hal_init.h
 * @brief   HAL 层统一初始化入口
 *
 * 本文件声明 HAL_BoardInit()，封装:
 *   - 系统时钟配置 (SystemClock_Config)
 *   - 外设公共时钟 (PeriphCommonClock_Config)
 *   - MPU 配置
 *   - 全部 MX_xxx_Init() 外设初始化
 *
 * 用法: 在 main() 中调用 HAL_BoardInit() 一次性完成所有硬件初始化。
 */

#ifndef HAL_INIT_H
#define HAL_INIT_H

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  板级硬件一键初始化
 * @note   调用顺序: 时钟 → MPU → 外设 (GPIO/DMA/SDRAM/LTDC/QSPI/...)
 *          本函数替代 main.c 中逐个调用的 MX_xxx_Init()
 */
void HAL_BoardInit(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_INIT_H */
