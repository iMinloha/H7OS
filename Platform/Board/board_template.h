/**
 * @file    board_template.h
 * @brief   New board config template / 新板子配置模板
 *
 * How to use / 使用方法:
 *   1. Copy this file → board_<name>.h / 复制此文件 → board_<板名>.h
 *   2. Replace BOARD_TEMPLATE with your macro (e.g. BOARD_MYBOARD)
 *      将 BOARD_TEMPLATE 替换为你的板子宏名 (如 BOARD_MYBOARD)
 *   3. Fill in MCU, peripheral switches, pin mappings
 *      填入 MCU 型号、外设开关、引脚映射
 *   4. Add #include "board_<name>.h" in board_select.h
 *      在 board_select.h 添加 #include "board_<板名>.h"
 *   5. Build: cmake -DBOARD=<name> / 编译: cmake -DBOARD=<板名>
 *
 * HAL board folder structure / 板子文件夹结构 (HAL/<name>/):
 *   Inc/     ← CubeMX generated headers / CubeMX 生成的头文件
 *   Src/     ← CubeMX generated sources (main.c auto-excluded)
 *              CubeMX 生成的源文件 (main.c 自动排除)
 *   Startup/ ← startup file / 启动文件
 *   BSP/     ← board BSP drivers / 板级 BSP 驱动
 *   *.ioc    ← CubeMX project / CubeMX 工程
 *   *.ld     ← linker script / 链接脚本
 */

#ifdef BOARD_TEMPLATE  /* ← replace with your BOARD_XXX macro / 替换为你的 BOARD_XXX 宏 */

/* ════════════════════════════════════════════════════════════
 *  Your Board — board config / 你的板子 — 板级配置
 * ════════════════════════════════════════════════════════════ */

#ifdef __cplusplus
extern "C" {
#endif

/* ── MCU identification / MCU 标识 ──────────────────────── */

#define BOARD_NAME              "YourBoard"
#define BOARD_MCU               STM32H743IIT6
#define BOARD_HSE_VALUE         ((uint32_t)25000000)   /* 25 MHz */
#define BOARD_LSE_VALUE         ((uint32_t)32768)      /* 32.768 kHz */

/* ── Peripheral switches (1=enable, 0=disable) / 外设开关 (1=启用) ── */

#define BOARD_HAS_GPIO          1
#define BOARD_HAS_DMA           1
#define BOARD_HAS_MDMA          0

/* Memory / 存储器 */
#define BOARD_HAS_SDRAM         0
#define BOARD_HAS_QSPI          0

/* Display / 显示 */
#define BOARD_HAS_LTDC          0
#define BOARD_HAS_DMA2D         0
#define BOARD_HAS_JPEG          0

/* Storage / 存储 */
#define BOARD_HAS_SDMMC         0

/* Serial / 串行 */
#define BOARD_HAS_USART1        1   /* at least one for Shell / 至少一个做 Shell */
#define BOARD_HAS_USART2        0
#define BOARD_HAS_I2C1          0
#define BOARD_HAS_USB_CDC       0

/* Analog / 模拟 */
#define BOARD_HAS_ADC1          0
#define BOARD_HAS_ADC3          0

/* Timer / 定时器 */
#define BOARD_HAS_TIM2          0

/* Misc / 杂项 */
#define BOARD_HAS_RNG           0
#define BOARD_HAS_RTC           0

/* ── Pin mappings / 引脚映射 ────────────────────────────── */

#define BSP_LED_PORT            GPIOB
#define BSP_LED_PIN             GPIO_PIN_0
#define BSP_SHELL_USART         USART1
#define BSP_SHELL_HUART         huart1
#define BSP_SHELL_IRQn          USART1_IRQn

/* ── External memory layout / 外部存储器布局 (if BOARD_HAS_SDRAM) ── */

#define SDRAM_BASE_ADDR         ((uint32_t)0xC0000000)
#define SDRAM_SIZE              ((uint32_t)(32 * 1024 * 1024))
#define VIDEO_MEM_SIZE          ((uint32_t)(8 * 1024 * 1024))
#define KERNEL_MEM_SIZE         ((uint32_t)(14 * 1024 * 1024))
#define USER_MEM_SIZE           ((uint32_t)(10 * 1024 * 1024))

#ifdef __cplusplus
}
#endif

#endif /* BOARD_TEMPLATE */
