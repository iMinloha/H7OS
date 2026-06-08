/**
 * @file    platform.c
 * @brief   Platform 抽象层实现 — 封装所有 HAL 调用
 *          Platform abstraction layer implementation — wraps all HAL calls
 *
 * 本文件是 Software ↔ HAL 之间的唯一桥梁。
 * 所有 HAL 句柄 (hsd1, hadc3, hqspi, hrtc, ...) 仅在此文件和 HAL/ 目录中使用。
 * Software 层通过 platform.h 中的函数间接访问硬件。
 *
 * This file is the ONLY bridge between Software and HAL.
 * All HAL handles (hsd1, hadc3, hqspi, hrtc, ...) are used ONLY here and in HAL/.
 * Software layer accesses hardware indirectly through functions declared in platform.h.
 */

#include "platform.h"
#include "board_select.h"

/* ── HAL 头文件 (仅 Platform 层引用) ──────────────────────── */
/*    HAL headers — only Platform layer includes these        */
#include "main.h"
#include "adc.h"
#include "sdmmc.h"
#include "quadspi.h"
#include "rtc.h"
#include "gpio.h"
#include "fatfs.h"
#include "stm32h7xx_hal.h"

/* ── 板级 BSP 函数 (由 HAL/<板名>/BSP/ 实现) ────────────────── */
/*    Board-level BSP functions (implemented in HAL/<board>/BSP/) */
extern void board_periph_init(void);
extern void devices_init(void);
extern void board_rtc_set_dfu_flag(void);

#if BOARD_HAS_SDMMC
#include "bsp_driver_sd.h"
#endif

/* ============================================================
 *  1. System
 * ============================================================ */

void Platform_DevicesInit(void)
{
    devices_init();
}

uint32_t Platform_GetSysClockFreq(void)
{
    return HAL_RCC_GetSysClockFreq();
}

void Platform_Reset(void)
{
    /* 保存后复位: 调用 NVIC_SystemReset() */
    NVIC_SystemReset();
}

/* ============================================================
 *  2. CPU / Temperature
 * ============================================================ */

void Platform_CPU_TempInit(void)
{
#if BOARD_HAS_ADC3
    HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
    HAL_ADC_Start(&hadc3);
#endif
}

float Platform_CPU_GetTemperature(void)
{
#if BOARD_HAS_ADC3
    HAL_ADC_Start(&hadc3);
    uint16_t adc_v = HAL_ADC_GetValue(&hadc3);

    /* STM32 出厂校准值 (地址固定, 参考 RM0433) */
    /* STM32 factory calibration values (fixed addresses, see RM0433) */
    uint16_t ts_cal1 = *(unsigned short *)(0x1FF1E820);
    uint16_t ts_cal2 = *(unsigned short *)(0x1FF1E840);

    double adc_slope = (110.0 - 30.0) / (double)(ts_cal2 - ts_cal1);
    return (float)(adc_slope * (adc_v - ts_cal1) + 30.0);
#else
    return 0.0f;
#endif
}

/* ============================================================
 *  3. GPIO
 * ============================================================ */

void Platform_GPIO_WritePin(void *port, uint16_t pin, uint8_t value)
{
    GPIO_PinState state = (value == 0) ? GPIO_PIN_RESET : GPIO_PIN_SET;
    HAL_GPIO_WritePin((GPIO_TypeDef *)port, pin, state);
}

uint8_t Platform_GPIO_ReadPin(void *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin((GPIO_TypeDef *)port, pin) == GPIO_PIN_SET) ? 1 : 0;
}

void Platform_GPIO_TogglePin(void *port, uint16_t pin)
{
    HAL_GPIO_TogglePin((GPIO_TypeDef *)port, pin);
}

/* ============================================================
 *  4. SD Card
 * ============================================================ */

int Platform_SD_IsPresent(void)
{
#if BOARD_HAS_SDMMC
    HAL_SD_CardInfoTypeDef info;
    if (HAL_SD_GetCardInfo(&hsd1, &info) != HAL_OK)
        return 0;
    uint64_t cap = (uint64_t)info.LogBlockNbr * (uint64_t)info.LogBlockSize;
    return (cap > 0) ? 1 : 0;
#else
    return 0;
#endif
}

uint64_t Platform_SD_GetCapacity(void)
{
#if BOARD_HAS_SDMMC
    HAL_SD_CardInfoTypeDef info;
    if (HAL_SD_GetCardInfo(&hsd1, &info) != HAL_OK)
        return 0;
    return (uint64_t)info.LogBlockNbr * (uint64_t)info.LogBlockSize;
#else
    return 0;
#endif
}

int Platform_SD_GetInfo(Platform_SD_Info *info)
{
#if BOARD_HAS_SDMMC
    if (!info) return 0;

    HAL_SD_CardInfoTypeDef hal_info;
    if (HAL_SD_GetCardInfo(&hsd1, &hal_info) != HAL_OK)
        return 0;

    info->capacity_bytes = (uint64_t)hal_info.LogBlockNbr * (uint64_t)hal_info.LogBlockSize;
    info->block_size     = hal_info.LogBlockSize;
    info->block_count    = hal_info.LogBlockNbr;
    info->card_type      = (uint8_t)hal_info.CardType;
    return 1;
#else
    return 0;
#endif
}

void *Platform_SD_GetFatFS(void)
{
#if BOARD_HAS_SDMMC
    extern FATFS SDFatFS;
    return &SDFatFS;
#else
    return NULL;
#endif
}

const char *Platform_SD_GetPath(void)
{
    return "0:";
}

/* ============================================================
 *  5. QSPI Flash
 * ============================================================ */

int Platform_QSPI_Init(void)
{
#if BOARD_HAS_QSPI
    return QSPI_W25Qxx_Init();
#else
    return -1;
#endif
}

uint32_t Platform_QSPI_ReadID(void)
{
#if BOARD_HAS_QSPI
    return QSPI_W25Qxx_ReadID();
#else
    return 0;
#endif
}

int Platform_QSPI_ChipErase(void)
{
#if BOARD_HAS_QSPI
    return QSPI_W25Qxx_ChipErase();
#else
    return -1;
#endif
}

int Platform_QSPI_Write(const uint8_t *data, uint32_t addr, uint32_t size)
{
#if BOARD_HAS_QSPI
    return QSPI_W25Qxx_WriteBuffer((uint8_t *)data, addr, size);
#else
    return -1;
#endif
}

int Platform_QSPI_Read(uint8_t *buf, uint32_t addr, uint32_t size)
{
#if BOARD_HAS_QSPI
    return QSPI_W25Qxx_ReadBuffer(buf, addr, size);
#else
    return -1;
#endif
}

void *Platform_QSPI_GetHandle(void)
{
#if BOARD_HAS_QSPI
    extern QSPI_HandleTypeDef hqspi;
    return &hqspi;
#else
    return NULL;
#endif
}

/* ============================================================
 *  6. RTC / DFU
 * ============================================================ */

void Platform_DFU_Request(void)
{
    Platform_DFU_SetFlag();
    Platform_Reset();
}

void Platform_DFU_SetFlag(void)
{
#if BOARD_HAS_RTC
    board_rtc_set_dfu_flag();
#endif
}

void Platform_DFU_ClearFlag(void)
{
#if BOARD_HAS_RTC
    extern RTC_HandleTypeDef hrtc;
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
#endif
}

/* ============================================================
 *  7. ADC
 * ============================================================ */

void Platform_ADC_CalibrateAndStart(void *hadc)
{
    if (!hadc) return;
    HAL_ADCEx_Calibration_Start((ADC_HandleTypeDef *)hadc, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);
    HAL_ADC_Start((ADC_HandleTypeDef *)hadc);
}

/* ============================================================
 *  8. Board Information
 * ============================================================ */

const char *Platform_GetBoardName(void)
{
    return BOARD_NAME;
}

int Platform_HasPeripheral(const char *peripheral)
{
    if (!peripheral) return 0;

    /* 字符串匹配外设开关 (精确匹配, 区分大小写) */
    /* String match peripheral switches (exact match, case-sensitive) */
    #define MATCH(name, macro) do { \
        const char *a = peripheral; const char *b = name; \
        while (*a && *b && *a == *b) { a++; b++; } \
        if (*a == '\0' && *b == '\0') return (macro); \
    } while(0)

    MATCH("GPIO",   BOARD_HAS_GPIO);
    MATCH("DMA",    BOARD_HAS_DMA);
    MATCH("MDMA",   BOARD_HAS_MDMA);
    MATCH("SDRAM",  BOARD_HAS_SDRAM);
    MATCH("QSPI",   BOARD_HAS_QSPI);
    MATCH("LTDC",   BOARD_HAS_LTDC);
    MATCH("DMA2D",  BOARD_HAS_DMA2D);
    MATCH("JPEG",   BOARD_HAS_JPEG);
    MATCH("SDMMC",  BOARD_HAS_SDMMC);
    MATCH("USART1", BOARD_HAS_USART1);
    MATCH("USART2", BOARD_HAS_USART2);
    MATCH("I2C1",   BOARD_HAS_I2C1);
    MATCH("USB_CDC",BOARD_HAS_USB_CDC);
    MATCH("ADC1",   BOARD_HAS_ADC1);
    MATCH("ADC3",   BOARD_HAS_ADC3);
    MATCH("TIM2",   BOARD_HAS_TIM2);
    MATCH("RNG",    BOARD_HAS_RNG);
    MATCH("RTC",    BOARD_HAS_RTC);

    #undef MATCH
    return 0;
}
