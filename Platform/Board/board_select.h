/**
 * @file    board_select.h
 * @brief   板级配置选择器
 *
 * 工作原理:
 *   1. CMakeLists.txt 根据 BOARD 变量定义 C 宏 (如 -DBOARD_FK743M2_IIT6)
 *   2. 本文件 include 所有板级配置头文件
 *   3. 每个 board_xxx.h 被 #ifdef BOARD_XXX 守卫, 仅匹配的板子生效
 *
 * 添加新板子:
 *   1. 在 HAL/ 下创建板子文件夹, CubeMX 生成代码
 *   2. 在本目录创建 board_<板名>.h, 用 #ifdef BOARD_XXX 守卫
 *   3. 在本文件添加 #include "board_<板名>.h"
 *   4. cmake -DBOARD=<板名> 编译
 */

#ifndef BOARD_SELECT_H
#define BOARD_SELECT_H

/* ── 所有板级配置 (仅匹配 CMake BOARD 宏的会生效) ───────── */

#include "board_fk743m2.h"

/* 新板子添加处:
   #include "board_yourboard.h"
*/

/* ── 外设可用性兼容宏 (未定义的默认为 0) ──────────── */

#ifndef BOARD_HAS_GPIO
    #define BOARD_HAS_GPIO      0
#endif
#ifndef BOARD_HAS_SDRAM
    #define BOARD_HAS_SDRAM     0
#endif
#ifndef BOARD_HAS_QSPI
    #define BOARD_HAS_QSPI      0
#endif
#ifndef BOARD_HAS_LTDC
    #define BOARD_HAS_LTDC      0
#endif
#ifndef BOARD_HAS_DMA2D
    #define BOARD_HAS_DMA2D     0
#endif
#ifndef BOARD_HAS_JPEG
    #define BOARD_HAS_JPEG      0
#endif
#ifndef BOARD_HAS_SDMMC
    #define BOARD_HAS_SDMMC     0
#endif
#ifndef BOARD_HAS_USART1
    #define BOARD_HAS_USART1    0
#endif
#ifndef BOARD_HAS_USART2
    #define BOARD_HAS_USART2    0
#endif
#ifndef BOARD_HAS_I2C1
    #define BOARD_HAS_I2C1      0
#endif
#ifndef BOARD_HAS_USB_CDC
    #define BOARD_HAS_USB_CDC   0
#endif
#ifndef BOARD_HAS_ADC1
    #define BOARD_HAS_ADC1      0
#endif
#ifndef BOARD_HAS_ADC3
    #define BOARD_HAS_ADC3      0
#endif
#ifndef BOARD_HAS_TIM2
    #define BOARD_HAS_TIM2      0
#endif
#ifndef BOARD_HAS_RNG
    #define BOARD_HAS_RNG       0
#endif
#ifndef BOARD_HAS_RTC
    #define BOARD_HAS_RTC       0
#endif

#endif /* BOARD_SELECT_H */
