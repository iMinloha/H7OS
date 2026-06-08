/**
 * @file    board_template.h
 * @brief   新板子配置模板
 *
 * 使用方法:
 *   1. 复制此文件 → board_<板名>.h
 *   2. 将 BOARD_TEMPLATE 替换为你的板子宏名 (如 BOARD_MYBOARD)
 *   3. 填入 MCU 型号、外设开关、引脚映射
 *   4. 在 board_select.h 添加 #include "board_<板名>.h"
 *   5. cmake -DBOARD=<板名> 编译 (CMake 自动将连字符转下划线)
 *
 * 板子文件夹结构 (HAL/<板名>/):
 *   Inc/     ← CubeMX 生成的头文件
 *   Src/     ← CubeMX 生成的源文件 (main.c 会被自动排除)
 *   Startup/ ← 启动文件
 *   BSP/     ← 板级 BSP 驱动 (bsp_devices.c, hal_init.c, 各外设驱动)
 *   <板名>.ioc ← CubeMX 工程文件
 *   STM32xxxxx_FLASH.ld ← 链接脚本
 */

#ifdef BOARD_TEMPLATE  /* ← 替换为你的 BOARD_XXX 宏 */
/* ════════════════════════════════════════════════════════════
 *  你的板子名称 — 板级配置
 * ════════════════════════════════════════════════════════════ */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 *  MCU 标识
 * ============================================================ */
#define BOARD_NAME              "YourBoard"
#define BOARD_MCU              STM32H743IIT6
#define BOARD_HSE_VALUE         ((uint32_t)25000000)
#define BOARD_LSE_VALUE         ((uint32_t)32768)

/* ============================================================
 *  外设开关 —— 1 启用, 0 禁用
 * ============================================================ */
#define BOARD_HAS_GPIO          1
#define BOARD_HAS_DMA           1
#define BOARD_HAS_MDMA          0

#define BOARD_HAS_SDRAM         0
#define BOARD_HAS_QSPI          0

#define BOARD_HAS_LTDC          0
#define BOARD_HAS_DMA2D         0
#define BOARD_HAS_JPEG          0

#define BOARD_HAS_SDMMC         0

#define BOARD_HAS_USART1        1   /* 至少一个 USART 做 Shell */
#define BOARD_HAS_USART2        0
#define BOARD_HAS_I2C1          0
#define BOARD_HAS_USB_CDC       0

#define BOARD_HAS_ADC1          0
#define BOARD_HAS_ADC3          0

#define BOARD_HAS_TIM2          0

#define BOARD_HAS_RNG           0
#define BOARD_HAS_RTC           0

/* ============================================================
 *  引脚映射
 * ============================================================ */
#define BSP_LED_PORT            GPIOB
#define BSP_LED_PIN             GPIO_PIN_0
#define BSP_SHELL_USART         USART1
#define BSP_SHELL_HUART         huart1
#define BSP_SHELL_IRQn          USART1_IRQn

/* ============================================================
 *  外部存储器布局 (仅在 BOARD_HAS_SDRAM=1 时有效)
 * ============================================================ */
#define SDRAM_BASE_ADDR         ((uint32_t)0xC0000000)
#define SDRAM_SIZE              ((uint32_t)(32 * 1024 * 1024))
#define VIDEO_MEM_SIZE          ((uint32_t)(8 * 1024 * 1024))
#define KERNEL_MEM_SIZE         ((uint32_t)(14 * 1024 * 1024))
#define USER_MEM_SIZE           ((uint32_t)(10 * 1024 * 1024))

#ifdef __cplusplus
}
#endif

#endif /* BOARD_TEMPLATE */
