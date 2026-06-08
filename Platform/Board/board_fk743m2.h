/**
 * @file    board_fk743m2.h
 * @brief   FK743M2-IIT6 板级配置
 *
 * 本文件定义 FK743M2-IIT6 开发板的:
 *   - MCU 型号与时钟源参数
 *   - 外设启用开关 (1=启用, 0=禁用)
 *   - 引脚映射
 *   - 外部存储器布局
 *
 * 添加新板子时, 复制此文件为 board_<板名>.h 并修改对应宏。
 */

#ifdef BOARD_FK743M2_IIT6
/* ════════════════════════════════════════════════════════════
 *  FK743M2-IIT6 板级配置
 *  仅当 CMake 定义 BOARD_FK743M2_IIT6 时生效
 * ════════════════════════════════════════════════════════════ */

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 *  MCU 标识
 * ============================================================ */

#define BOARD_NAME              "FK743M2-IIT6"
#define BOARD_MCU              STM32H743IIT6
#define BOARD_HSE_VALUE         ((uint32_t)25000000)   /* 25 MHz */
#define BOARD_LSE_VALUE         ((uint32_t)32768)      /* 32.768 kHz */

/* ============================================================
 *  外设开关 —— 定义 1 启用, 0 禁用
 * ============================================================ */

/* --- 核心外设 (通常总是启用) --- */
#define BOARD_HAS_GPIO          1
#define BOARD_HAS_DMA           1
#define BOARD_HAS_MDMA          1

/* --- 存储器 --- */
#define BOARD_HAS_SDRAM         1   /* FMC SDRAM, 32 MB @ 0xC0000000 */
#define BOARD_HAS_QSPI          1   /* W25Qxx QSPI Flash, 16 MB */

/* --- 显示 --- */
#define BOARD_HAS_LTDC          1   /* RGB565 LCD, 480x272 */
#define BOARD_HAS_DMA2D         1   /* Chrom-ART 图形加速 */
#define BOARD_HAS_JPEG          1   /* 硬件 JPEG 编解码 */

/* --- 存储接口 --- */
#define BOARD_HAS_SDMMC         1   /* SDMMC1, 4-bit SD 卡 */

/* --- 串行通信 --- */
#define BOARD_HAS_USART1        1   /* USB CDC Shell 主控台 */
#define BOARD_HAS_USART2        1   /* 辅助串口 */
#define BOARD_HAS_I2C1          1   /* 触摸屏 + 扩展 I2C */
#define BOARD_HAS_USB_CDC       1   /* USB CDC 虚拟串口 */

/* --- 模拟 --- */
#define BOARD_HAS_ADC1          1   /* 外部 ADC 输入 (PC4) */
#define BOARD_HAS_ADC3          1   /* 内部温度传感器 */

/* --- 定时器 --- */
#define BOARD_HAS_TIM2          1   /* 双通道 PWM (PA0, PA1) */

/* --- 安全 / 杂项 --- */
#define BOARD_HAS_RNG           1   /* 硬件随机数 */
#define BOARD_HAS_RTC           1   /* RTC (含 DFU 备份寄存器) */

/* ============================================================
 *  引脚映射 —— 用于 BSP 驱动
 * ============================================================ */

/* 用户 LED */
#define BSP_LED_PORT            GPIOH
#define BSP_LED_PIN             GPIO_PIN_7

/* Shell 控制台 USART */
#define BSP_SHELL_USART         USART1
#define BSP_SHELL_HUART         huart1
#define BSP_SHELL_IRQn          USART1_IRQn

/* SD 卡检测 (如果硬件支持) */
// #define BSP_SD_DETECT_PORT   GPIOx
// #define BSP_SD_DETECT_PIN    GPIO_PIN_x

/* ============================================================
 *  外部存储器布局
 * ============================================================ */

#define SDRAM_BASE_ADDR         ((uint32_t)0xC0000000)
#define SDRAM_SIZE              ((uint32_t)(32 * 1024 * 1024))   /* 32 MB */

/* SDRAM 分区 (共 32 MB) */
#define VIDEO_MEM_SIZE          ((uint32_t)(8 * 1024 * 1024))    /* 8 MB LTDC 帧缓冲 */
#define KERNEL_MEM_SIZE         ((uint32_t)(14 * 1024 * 1024))   /* 14 MB 内核 TLSF 池 */
#define USER_MEM_SIZE           ((uint32_t)(10 * 1024 * 1024))   /* 10 MB 用户 TLSF 池 */

#define VIDEO_MEM_ADDR          SDRAM_BASE_ADDR
#define KERNEL_MEM_ADDR         (SDRAM_BASE_ADDR + VIDEO_MEM_SIZE)
#define USER_MEM_ADDR           (KERNEL_MEM_ADDR + KERNEL_MEM_SIZE)

/* QSPI Flash */
#define QSPI_FLASH_SIZE         ((uint32_t)(16 * 1024 * 1024))   /* 16 MB */
#define QSPI_MEM_ADDR           ((uint32_t)0x90000000)

#ifdef __cplusplus
}
#endif

#endif /* BOARD_FK743M2_IIT6 */
