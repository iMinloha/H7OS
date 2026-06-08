/**
 * @file    hal_init.h
 * @brief   HAL layer unified init entry / HAL 层统一初始化入口
 *
 * Extracts from CubeMX-generated main.c:
 * 从 CubeMX 生成的 main.c 中提取:
 *   - SystemClock_Config()
 *   - MPU_Config()
 *   - all MX_xxx_Init() calls / 全部 MX_xxx_Init() 调用
 *
 * Wraps them into HAL_BoardInit(). / 封装为 HAL_BoardInit().
 */

#ifndef HAL_INIT_H
#define HAL_INIT_H

#include "main.h"

/** One-call hardware init / 一键硬件初始化 */
void HAL_BoardInit(void);

#endif
