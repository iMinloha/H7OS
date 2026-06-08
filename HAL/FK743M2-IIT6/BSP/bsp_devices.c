/**
 * @file    bsp_devices.c
 * @brief   FK743M2-IIT6 板级设备注册
 *
 * 实现 Platform 层要求的两个函数:
 *   - devices_init()        —— 将板载外设注册到 DrT 设备树
 *   - board_periph_init()   —— 板级外设硬件初始化 (SD/Touch/USB)
 *
 * 使用 board_fk743m2.h 中的 BOARD_HAS_xxx 宏进行条件编译。
 */

#include "bsp_devices.h"
#include "board_select.h"
#include "main.h"

/* ── 板级 BSP 驱动头文件 ────────────────────────────────── */
#if BOARD_HAS_CPU
#include "cpu/bsp_cpu.h"
#endif
#if BOARD_HAS_USART1
#include "usart/bsp_usart.h"
#endif
#if BOARD_HAS_GPIO
#include "gpio/bsp_gpio.h"
#endif
#if BOARD_HAS_TIM2
#include "pwm/bsp_pwm.h"
#endif
#if BOARD_HAS_I2C1
#include "i2c/bsp_i2c.h"
#endif
#if BOARD_HAS_ADC1
#include "adc/bsp_adc.h"
#endif

/* ── 板级外设硬件驱动 ────────────────────────────────────── */
#if BOARD_HAS_SDMMC
#include "sdmmc.h"
#include "bsp_driver_sd.h"
#endif
#if BOARD_HAS_USB_CDC
#include "usb_device.h"
#endif
#include "torch_iic.h"     /* 触摸屏 I2C 驱动 */

#if BOARD_HAS_RTC
#include "rtc.h"
#endif

/* ============================================================
 *  board_periph_init —— 板级外设硬件初始化
 * ============================================================ */

void board_periph_init(void)
{
#if BOARD_HAS_RTC
    /* DFU boot check: read RTC backup register (preserved across reset) */
    extern void jump_to_bootloader(void);
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0x44465501) {
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0);  /* Clear flag */
        jump_to_bootloader();  /* Jump to STM32 system bootloader (USB DFU) */
    }
#endif

#if BOARD_HAS_USB_CDC
    MX_USB_DEVICE_Init();
#endif

#if BOARD_HAS_SDMMC
    BSP_SD_Init();
#endif

#if BOARD_HAS_I2C1
    Touch_Init();
#endif
}

/* ============================================================
 *  devices_init —— 将外设注册到 DrT 设备树
 * ============================================================ */

void devices_init(void)
{
#if BOARD_HAS_CPU
    cpu_device_init();
#endif
#if BOARD_HAS_USART1
    usart1_device_init();
#endif
#if BOARD_HAS_GPIO
    gpio_device_init();
#endif
#if BOARD_HAS_TIM2
    pwm_device_init();
#endif
#if BOARD_HAS_I2C1
    i2c_device_init();
#endif
#if BOARD_HAS_ADC1
    adc_device_init();
#endif
}
