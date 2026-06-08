/**
 * @file    platform.h
 * @brief   Platform 抽象层 — Software 层唯一硬件接口
 *          Platform Abstraction Layer — the ONLY hardware interface for Software layer
 *
 * 设计原则 / Design Principles:
 *   - Software 层只 #include 本文件, 绝不直接引用 HAL 头文件
 *     Software layer ONLY includes this file, NEVER includes HAL headers directly
 *   - 所有 HAL 句柄 (huart, hsd, hadc, ...) 对 Software 层不可见
 *     All HAL handles are opaque to the Software layer
 *   - 本层封装所有板级差异, 切换板子只需修改 CMake BOARD 变量
 *     All board differences are encapsulated; switch boards via cmake -DBOARD=xxx
 *
 * 调用链 / Call Chain:
 *   Software (Applications, Command, RAMFS) → Platform API → HAL/BSP
 *
 * 用法 / Usage:
 *   #include "platform.h"  // 替代所有 HAL 头文件 / replaces ALL HAL headers
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include "bsp_file_ops.h"       /* bsp_file_ops_t for device I/O */

/* ── CMSIS 设备头文件 (仅寄存器定义, 非 HAL 驱动) ──────────── */
/*    CMSIS device header (register definitions only, NOT HAL driver) */
/*    提供 GPIOx, USARTx, RCC 等基地址宏, 供板级配置使用         */
/*    Provides GPIOx, USARTx, RCC base address macros for board configs */
#include "stm32h743xx.h"

/* ── GPIO 引脚位掩码 (来自 HAL, 仅常量定义, 不引入 HAL 函数) ── */
/*    GPIO pin bit masks (from HAL, constants only, no HAL functions)   */
#ifndef GPIO_PIN_0
#define GPIO_PIN_0       ((uint16_t)0x0001)
#define GPIO_PIN_1       ((uint16_t)0x0002)
#define GPIO_PIN_2       ((uint16_t)0x0004)
#define GPIO_PIN_3       ((uint16_t)0x0008)
#define GPIO_PIN_4       ((uint16_t)0x0010)
#define GPIO_PIN_5       ((uint16_t)0x0020)
#define GPIO_PIN_6       ((uint16_t)0x0040)
#define GPIO_PIN_7       ((uint16_t)0x0080)
#define GPIO_PIN_8       ((uint16_t)0x0100)
#define GPIO_PIN_9       ((uint16_t)0x0200)
#define GPIO_PIN_10      ((uint16_t)0x0400)
#define GPIO_PIN_11      ((uint16_t)0x0800)
#define GPIO_PIN_12      ((uint16_t)0x1000)
#define GPIO_PIN_13      ((uint16_t)0x2000)
#define GPIO_PIN_14      ((uint16_t)0x4000)
#define GPIO_PIN_15      ((uint16_t)0x8000)
#define GPIO_PIN_ALL     ((uint16_t)0xFFFF)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 *  1. System / 系统
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  Platform 层统一初始化 (在 HAL_BoardInit() 之后调用)
 *         Unified Platform initialization (call after HAL_BoardInit())
 *
 * 执行: board_periph_init() → 板级外设初始化 (SD卡检测, 触摸屏, USB CDC, DFU检测)
 * Does: board_periph_init() → board-level peripheral init (SD detect, Touch, USB CDC, DFU check)
 */
void Platform_Init(void);

/**
 * @brief  将所有板载外设注册到 DrT 设备树 (在 taskGlobalInit 中调用)
 *         Register all onboard peripherals into DrT device tree (called from taskGlobalInit)
 *
 * 执行: 各 BSP 驱动的 xxx_device_init() (cpu, usart, gpio, pwm, adc, i2c)
 * Does: each BSP driver's xxx_device_init() (cpu, usart, gpio, pwm, adc, i2c)
 */
void Platform_DevicesInit(void);

/**
 * @brief  获取系统时钟频率 (Hz)
 *         Get system clock frequency (Hz)
 * @return HCLK frequency in Hz / HCLK 频率 (Hz)
 */
uint32_t Platform_GetSysClockFreq(void);

/**
 * @brief  系统复位 (保存文件系统后调用)
 *         System reset (call after saving filesystem)
 */
void Platform_Reset(void);

/* ═══════════════════════════════════════════════════════════════════════════
 *  2. CPU / 处理器信息
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  初始化 MCU 内部温度传感器 (ADC3)
 *         Initialize MCU internal temperature sensor (ADC3)
 *
 * 校准 ADC3 并启动连续转换, 之后可通过 Platform_CPU_GetTemperature() 读取温度。
 * Calibrates ADC3 and starts continuous conversion;
 * call Platform_CPU_GetTemperature() to read temperature afterwards.
 */
void Platform_CPU_TempInit(void);

/**
 * @brief  读取 MCU 内部温度 (°C)
 *         Read MCU internal temperature (°C)
 *
 * 使用 STM32 出厂校准值 (TS_CAL1/TS_CAL2) 进行线性插值。
 * Uses STM32 factory calibration values (TS_CAL1/TS_CAL2) for linear interpolation.
 *
 * @return Temperature in °C / 温度 (°C)
 */
float Platform_CPU_GetTemperature(void);

/* ═══════════════════════════════════════════════════════════════════════════
 *  3. GPIO / 通用输入输出
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  写 GPIO 引脚 (高/低)
 *         Write GPIO pin (high/low)
 * @param  port  GPIO 端口基地址 (如 GPIOH) / GPIO port base address (e.g. GPIOH)
 * @param  pin   引脚号 (如 GPIO_PIN_7) / pin number (e.g. GPIO_PIN_7)
 * @param  value 0=低电平/low, 非0=高电平/high
 */
void Platform_GPIO_WritePin(void *port, uint16_t pin, uint8_t value);

/**
 * @brief  读 GPIO 引脚
 *         Read GPIO pin
 * @param  port  GPIO 端口基地址 / GPIO port base address
 * @param  pin   引脚号 / pin number
 * @return 0=低电平/low, 1=高电平/high
 */
uint8_t Platform_GPIO_ReadPin(void *port, uint16_t pin);

/**
 * @brief  翻转 GPIO 引脚电平
 *         Toggle GPIO pin level
 * @param  port  GPIO 端口基地址 / GPIO port base address
 * @param  pin   引脚号 / pin number
 */
void Platform_GPIO_TogglePin(void *port, uint16_t pin);

/* ═══════════════════════════════════════════════════════════════════════════
 *  4. SD Card / SD 卡
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  SD 卡信息结构 (Platform 层抽象, 不暴露 HAL 类型)
 *         SD card info struct (Platform abstraction, no HAL types exposed)
 */
typedef struct {
    uint64_t capacity_bytes;    /* 总容量 (字节) / total capacity (bytes) */
    uint32_t block_size;        /* 块大小 (字节) / block size (bytes) */
    uint32_t block_count;       /* 块数量 / block count */
    uint8_t  card_type;         /* 卡类型 / card type: 0=unknown, 1=SDSC, 2=SDHC, 3=SDXC */
} Platform_SD_Info;

/**
 * @brief  检测 SD 卡是否在位
 *         Check if SD card is present
 * @return 1=SD卡在位/present, 0=未检测到/not found
 */
int Platform_SD_IsPresent(void);

/**
 * @brief  获取 SD 卡容量 (字节)
 *         Get SD card capacity (bytes)
 * @return 容量 (字节), 0 表示无卡或读取失败 / capacity (bytes), 0 = no card or read error
 */
uint64_t Platform_SD_GetCapacity(void);

/**
 * @brief  获取 SD 卡详细信息
 *         Get SD card detailed info
 * @param  info 输出参数 / output parameter
 * @return 1=成功/success, 0=失败/failure
 */
int Platform_SD_GetInfo(Platform_SD_Info *info);

/**
 * @brief  获取 SD 卡 FATFS 对象指针 (供 f_mount 等使用)
 *         Get SD card FATFS object pointer (for f_mount etc.)
 * @return FATFS* 指针, NULL 表示无卡 / FATFS* pointer, NULL = no card
 * @note   仅用于挂载操作, 不要直接操作该对象 / Only for mount operations
 */
void *Platform_SD_GetFatFS(void);

/**
 * @brief  获取 SD 卡路径字符串 (供 f_mount 使用)
 *         Get SD card path string (for f_mount)
 * @return "0:" 路径 / "0:" path string
 */
const char *Platform_SD_GetPath(void);

/* ═══════════════════════════════════════════════════════════════════════════
 *  5. QSPI Flash / QSPI 闪存
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  初始化 QSPI Flash (W25Qxx)
 *         Initialize QSPI Flash (W25Qxx)
 * @return 0=成功/success, 非0=失败/failure
 */
int Platform_QSPI_Init(void);

/**
 * @brief  读取 QSPI Flash 芯片 ID
 *         Read QSPI Flash chip ID
 * @return 24-bit JEDEC ID (0xEF4017 = W25Q64)
 */
uint32_t Platform_QSPI_ReadID(void);

/**
 * @brief  全片擦除 QSPI Flash
 *         Chip erase QSPI Flash
 * @return 0=成功/success, 非0=失败/failure
 */
int Platform_QSPI_ChipErase(void);

/**
 * @brief  写数据到 QSPI Flash
 *         Write data to QSPI Flash
 * @param  data       数据缓冲区 / data buffer
 * @param  addr       写入地址 (Flash 偏移) / write address (Flash offset)
 * @param  size       数据大小 (字节) / data size (bytes)
 * @return 0=成功/success, 非0=失败/failure
 */
int Platform_QSPI_Write(const uint8_t *data, uint32_t addr, uint32_t size);

/**
 * @brief  从 QSPI Flash 读数据
 *         Read data from QSPI Flash
 * @param  buf        读取缓冲区 / read buffer
 * @param  addr       读取地址 (Flash 偏移) / read address (Flash offset)
 * @param  size       读取大小 (字节) / read size (bytes)
 * @return 0=成功/success, 非0=失败/failure
 */
int Platform_QSPI_Read(uint8_t *buf, uint32_t addr, uint32_t size);

/**
 * @brief  获取 QSPI 句柄 (供 addDevice 注册到 DrT)
 *         Get QSPI handle (for addDevice registration to DrT)
 * @return QSPI_HandleTypeDef* 指针 / QSPI_HandleTypeDef* pointer
 * @note   仅用于设备树注册, Software 层不应直接操作此句柄
 *         Only for device tree registration; Software should not use this handle directly
 */
void *Platform_QSPI_GetHandle(void);

/* ═══════════════════════════════════════════════════════════════════════════
 *  6. RTC / DFU / 实时时钟与固件升级
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  请求进入 DFU 模式 (设置 RTC 备份寄存器标记后复位)
 *         Request DFU mode (set RTC backup register flag then reset)
 *
 * 调用后 MCU 将复位, boot 时检测到 DFU 标记后跳入系统 bootloader。
 * After calling, MCU resets; on boot, DFU flag triggers jump to system bootloader.
 */
void Platform_DFU_Request(void);

/**
 * @brief  设置 DFU 标记 (不复位, 下次复位后进入 DFU)
 *         Set DFU flag (no reset; enters DFU on next reset)
 */
void Platform_DFU_SetFlag(void);

/**
 * @brief  清除 DFU 标记
 *         Clear DFU flag
 */
void Platform_DFU_ClearFlag(void);

/* ═══════════════════════════════════════════════════════════════════════════
 *  7. ADC / 模数转换
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  校准并启动 ADC
 *         Calibrate and start ADC
 * @param  hadc ADC 句柄 (由 BSP 层管理) / ADC handle (managed by BSP layer)
 * @note   Software 层应通过 Platform_DevicesInit() 间接使用, 而非直接调用此函数
 *         Software layer should use indirectly via Platform_DevicesInit(), not call this directly
 */
void Platform_ADC_CalibrateAndStart(void *hadc);

/* ═══════════════════════════════════════════════════════════════════════════
 *  8. Board Information / 板级信息
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief  获取板卡名称
 *         Get board name string
 * @return 板卡名称 (如 "FK743M2-IIT6") / board name (e.g. "FK743M2-IIT6")
 */
const char *Platform_GetBoardName(void);

/**
 * @brief  检查外设是否启用
 *         Check if a peripheral is enabled on this board
 * @param  peripheral 外设名称 (如 "SDMMC", "LTDC", "QSPI") / peripheral name
 * @return 1=启用/enabled, 0=禁用/disabled
 */
int Platform_HasPeripheral(const char *peripheral);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_H */
