/**
 * @file    bsp_devices.c
 * @brief   板级 BSP 设备注册
 *
 * 实现 Platform 层要求的函数:
 *   - devices_init()        —— 将板载外设注册到 DrT 设备树
 *   - board_periph_init()   —— 板级外设硬件初始化 (SD/Touch/USB)
 *   - board_sd_get_capacity()
 *   - board_rtc_set_dfu_flag()
 */

#include "bsp_devices.h"
#include "board_select.h"
#include "main.h"

/* ── 板级 BSP 驱动头文件 ────────────────────────────────── */
#include "gpio/bsp_gpio.h"
#include "usart/bsp_usart.h"
// #include "pwm/bsp_pwm.h"
// #include "adc/bsp_adc.h"
// #include "i2c/bsp_i2c.h"
// #include "cpu/bsp_cpu.h"

#include "sdmmc.h"
#include "bsp_driver_sd.h"
#include "usb_device.h"
#include "rtc.h"
// #include "torch_iic.h"

/* ============================================================
 *  board_periph_init
 * ============================================================ */

void board_periph_init(void)
{
#if BOARD_HAS_RTC
    extern void jump_to_bootloader(void);
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0x44465501) {
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);
        jump_to_bootloader();
    }
#endif
#if BOARD_HAS_USB_CDC
    MX_USB_DEVICE_Init();
#endif
#if BOARD_HAS_SDMMC
    BSP_SD_Init();
#endif
// #if BOARD_HAS_TOUCH
//     Touch_Init();
// #endif
}

/* ============================================================
 *  devices_init
 * ============================================================ */

void devices_init(void)
{
#if BOARD_HAS_GPIO
    gpio_device_init();
#endif
#if BOARD_HAS_USART1
    usart1_device_init();
#endif
// #if BOARD_HAS_PWM
//     pwm_device_init();
// #endif
}

/* ============================================================
 *  Platform API wrappers
 * ============================================================ */

#if BOARD_HAS_SDMMC
uint64_t board_sd_get_capacity(void)
{
    HAL_SD_CardInfoTypeDef info;
    if (HAL_SD_GetCardInfo(&hsd1, &info) != HAL_OK)
        return 0;
    return (uint64_t) info.LogBlockNbr * (uint64_t) info.LogBlockSize;
}
#else
uint64_t board_sd_get_capacity(void) { return 0; }
#endif

#if BOARD_HAS_RTC
void board_rtc_set_dfu_flag(void)
{
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x44465501);
}
#else
void board_rtc_set_dfu_flag(void) { }
#endif
