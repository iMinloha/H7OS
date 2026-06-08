/**
 * @file    hal_init.c
 * @brief   HAL 层统一初始化实现 —— 从 CubeMX 生成的 main.c 提取
 *
 * 每次 CubeMX 重新生成代码后，需检查同步:
 *   1. SystemClock_Config() —— 如有修改时钟树，从新 main.c 抄过来
 *   2. MPU_Config()          —— 如有修改 MPU，从新 main.c 抄过来
 *   3. MX_xxx_Init() 调用列表 —— 如有增删外设，增删对应行
 */

#include "hal_init.h"

/* ── 外设头文件 (声明 MX_xxx_Init) ─────────────────────── */
#include "gpio.h"      /* 示例: 按实际外设增删 */
// #include "usart.h"
// #include "adc.h"
// #include "sdmmc.h"
// #include "fatfs.h"
// ...

/* ── 静态函数声明 ──────────────────────────────────────── */
static void SystemClock_Config(void);
static void MPU_Config(void);

/* ============================================================
 *  HAL_BoardInit
 * ============================================================ */

void HAL_BoardInit(void)
{
    MPU_Config();
    SystemClock_Config();

    /* 按 CubeMX 生成的 main.c 中的调用顺序列出 */
    MX_GPIO_Init();
    // MX_DMA_Init();
    // MX_USART1_UART_Init();
    // MX_SDMMC1_SD_Init();
    // MX_FATFS_Init();
    // ...
}

/* ============================================================
 *  时钟配置 —— 从 CubeMX main.c 的 SystemClock_Config() 复制
 * ============================================================ */

static void SystemClock_Config(void)
{
    /* TODO: 从 CubeMX 生成的 main.c 复制 */
}

/* ============================================================
 *  MPU 配置 —— 从 CubeMX main.c 的 MPU_Config() 复制
 * ============================================================ */

static void MPU_Config(void)
{
    /* TODO: 从 CubeMX 生成的 main.c 复制 */
}
