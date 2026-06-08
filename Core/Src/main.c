/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body — 通用应用入口
  ******************************************************************************
  *
  * 本文件是 H7OS 的通用 main() 入口，不包含任何板级特定代码。
  *
  * 初始化流程:
  *   1. Cache 使能 (I-Cache + D-Cache)
  *   2. HAL_Init()            —— HAL 库初始化
  *   3. HAL_BoardInit()       —— 板级硬件初始化 (时钟/MPU/全部外设)
  *   4. Platform_Init()       —— 外设注册 + 板级外设初始化
  *   5. MX_FREERTOS_Init()    —— FreeRTOS 内核 + OS 任务创建
  *   6. osKernelStart()       —— 启动调度器
  *
  * 板级差异完全封装在 HAL/<板名>/ 和 Platform/ 层中。
  * 切换板子只需修改 CMakeLists.txt 中的 BOARD 变量。
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "hal_init.h"
#include "bsp_init.h"
#include "cmsis_os.h"

/* Private function prototypes -----------------------------------------------*/
void MX_FREERTOS_Init(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* Enable I-Cache & D-Cache */
    SCB_EnableICache();
    SCB_EnableDCache();

    /* HAL 库初始化 (复位外设、配置 Flash、SysTick) */
    HAL_Init();

    /* HAL 层: 板级硬件一键初始化 (时钟树 + MPU + 全部外设 MX_xxx_Init) */
    HAL_BoardInit();

    /* Platform 层: 外设注册到 DrT + 板级外设初始化 (SD/Touch/USB) */
    Platform_Init();

    /* FreeRTOS 初始化 (内核对象 + OS 任务创建) */
    MX_FREERTOS_Init();

    /* 启动 FreeRTOS 调度器 (永不返回) */
    osKernelStart();

    /* 不应该到达这里 */
    while (1);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
