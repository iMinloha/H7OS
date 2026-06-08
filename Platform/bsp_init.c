/**
 * @file    bsp_init.c
 * @brief   Platform 层统一初始化实现
 *
 * 本文件实现 Platform_Init()，它调用:
 *   1. board_periph_init() —— 板级外设初始化 (SD/Touch/USB 等, 由板级 BSP 提供)
 *   2. devices_init()      —— 将所有板载外设注册到 DrT 设备树 (由板级 BSP 提供)
 *
 * 这两个函数的具体实现位于 HAL/<板名>/BSP/ 中, 通过链接器与 Platform 层对接。
 * Platform 层本身不包含任何板级特定代码。
 */

#include "bsp_init.h"
#include "bsp_devices.h"

/* ── 板级 BSP 提供的函数 (实现在 HAL/<板名>/BSP/) ────────── */

/**
 * @brief 板级外设初始化 (SD卡、触摸屏、USB CDC 等)
 * @note  由各板子的 BSP 实现, Platform 层仅声明接口
 */
extern void board_periph_init(void);

/**
 * @brief 将板载外设注册到 DrT 设备树
 * @note  由各板子的 BSP 实现, 通过 DEV_REGISTER / GPIO_OUTPUT 等宏注册
 */
extern void devices_init(void);

/* ============================================================
 *  Platform_Init —— 统一入口
 * ============================================================ */

void Platform_Init(void)
{
    /* 板级外设硬件初始化 (SD卡检测、触摸屏、USB CDC)
     * 注意: 此时 DrT 和 TLSF 尚未初始化, 不能调用 addDevice()
     * devices_init() 在 MX_FREERTOS_Init() 中 taskGlobalInit() 之后调用 */
    board_periph_init();
}
