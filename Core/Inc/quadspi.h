/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    quadspi.h
  * @brief   This file contains all the function prototypes for
  *          the quadspi.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __QUADSPI_H__
#define __QUADSPI_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

extern QSPI_HandleTypeDef hqspi;

/* USER CODE BEGIN Private defines */

#define QSPI_W25Qxx_OK           		0		// W25Qxx通信正常
#define W25Qxx_ERROR_INIT         		-1		// 初始化错误
#define W25Qxx_ERROR_WriteEnable       -2		// 写使能错误
#define W25Qxx_ERROR_AUTOPOLLING       -3		// 轮询等待错误，无响应
#define W25Qxx_ERROR_Erase         		-4		// 擦除错误
#define W25Qxx_ERROR_TRANSMIT         	-5		// 传输错误
#define W25Qxx_ERROR_MemoryMapped		-6    // 内存映射模式错误

#define W25Qxx_CMD_EnableReset  		0x66		// 使能复位
#define W25Qxx_CMD_ResetDevice   	0x99		// 复位器件
#define W25Qxx_CMD_JedecID 			0x9F		// JEDEC ID
#define W25Qxx_CMD_WriteEnable		0X06		// 写使能

#define W25Qxx_CMD_SectorErase 		0x20		// 扇区擦除，4K字节， 参考擦除时间 45ms
#define W25Qxx_CMD_BlockErase_32K 	0x52		// 块擦除，  32K字节，参考擦除时间 120ms
#define W25Qxx_CMD_BlockErase_64K 	0xD8		// 块擦除，  64K字节，参考擦除时间 150ms
#define W25Qxx_CMD_ChipErase 			0xC7		// 整片擦除，参考擦除时间 20S

#define W25Qxx_CMD_QuadInputPageProgram  	0x32  		// 1-1-4模式下(1线指令1线地址4线数据)，页编程指令，参考写入时间 0.4ms
#define W25Qxx_CMD_FastReadQuad_IO       	0xEB  		// 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令

#define W25Qxx_CMD_ReadStatus_REG1			0X05			// 读状态寄存器1
#define W25Qxx_Status_REG1_BUSY  			0x01			// 读状态寄存器1的第0位（只读），Busy标志位，当正在擦除/写入数据/写命令时会被置1
#define W25Qxx_Status_REG1_WEL  				0x02			// 读状态寄存器1的第1位（只读），WEL写使能标志位，该标志位为1时，代表可以进行写操作

#define W25Qxx_PageSize       				128			// 页大小，256字节
#define W25Qxx_FlashSize       				0x800000		// W25Q64大小，8M字节
#define W25Qxx_FLASH_ID           			0Xef4017    // W25Q64 JEDEC ID
#define W25Qxx_ChipErase_TIMEOUT_MAX		100000U		// 超时等待时间，W25Q64整片擦除所需最大时间是100S
#define W25Qxx_Mem_Addr						0x90000000 	// 内存映射模式的地址


#define QUADSPI_CLK_PIN							GPIO_PIN_2								// QUADSPI_CLK 锟斤拷锟斤拷
#define	QUADSPI_CLK_PORT							GPIOB										// QUADSPI_CLK 锟斤拷锟脚端匡拷
#define	QUADSPI_CLK_AF								GPIO_AF9_QUADSPI						// QUADSPI_CLK IO锟节革拷锟斤拷
#define GPIO_QUADSPI_CLK_ENABLE      			__HAL_RCC_GPIOB_CLK_ENABLE()	 	// QUADSPI_CLK 锟斤拷锟斤拷时锟斤拷

#define QUADSPI_BK1_NCS_PIN						GPIO_PIN_6								// QUADSPI_BK1_NCS 锟斤拷锟斤拷
#define	QUADSPI_BK1_NCS_PORT						GPIOB										// QUADSPI_BK1_NCS 锟斤拷锟脚端匡拷
#define	QUADSPI_BK1_NCS_AF						GPIO_AF10_QUADSPI						// QUADSPI_BK1_NCS IO锟节革拷锟斤拷
#define GPIO_QUADSPI_BK1_NCS_ENABLE        	__HAL_RCC_GPIOB_CLK_ENABLE()	 	// QUADSPI_BK1_NCS 锟斤拷锟斤拷时锟斤拷

#define QUADSPI_BK1_IO0_PIN						GPIO_PIN_11								// QUADSPI_BK1_IO0 锟斤拷锟斤拷
#define	QUADSPI_BK1_IO0_PORT						GPIOD										// QUADSPI_BK1_IO0 锟斤拷锟脚端匡拷
#define	QUADSPI_BK1_IO0_AF						GPIO_AF9_QUADSPI						// QUADSPI_BK1_IO0 IO锟节革拷锟斤拷
#define GPIO_QUADSPI_BK1_IO0_ENABLE        	__HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO0 锟斤拷锟斤拷时锟斤拷

#define QUADSPI_BK1_IO1_PIN						GPIO_PIN_12								// QUADSPI_BK1_IO1 锟斤拷锟斤拷
#define	QUADSPI_BK1_IO1_PORT						GPIOD										// QUADSPI_BK1_IO1 锟斤拷锟脚端匡拷
#define	QUADSPI_BK1_IO1_AF						GPIO_AF9_QUADSPI						// QUADSPI_BK1_IO1 IO锟节革拷锟斤拷
#define GPIO_QUADSPI_BK1_IO1_ENABLE        	__HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO1 锟斤拷锟斤拷时锟斤拷

#define QUADSPI_BK1_IO2_PIN						GPIO_PIN_2								// QUADSPI_BK1_IO2 锟斤拷锟斤拷
#define	QUADSPI_BK1_IO2_PORT						GPIOE										// QUADSPI_BK1_IO2 锟斤拷锟脚端匡拷
#define	QUADSPI_BK1_IO2_AF						GPIO_AF9_QUADSPI						// QUADSPI_BK1_IO2 IO锟节革拷锟斤拷
#define GPIO_QUADSPI_BK1_IO2_ENABLE        	__HAL_RCC_GPIOE_CLK_ENABLE()	 	// QUADSPI_BK1_IO2 锟斤拷锟斤拷时锟斤拷

#define QUADSPI_BK1_IO3_PIN						GPIO_PIN_13								// QUADSPI_BK1_IO3 锟斤拷锟斤拷
#define	QUADSPI_BK1_IO3_PORT						GPIOD										// QUADSPI_BK1_IO3 锟斤拷锟脚端匡拷
#define	QUADSPI_BK1_IO3_AF						GPIO_AF9_QUADSPI						// QUADSPI_BK1_IO3 IO锟节革拷锟斤拷
#define GPIO_QUADSPI_BK1_IO3_ENABLE      	__HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO3 锟斤拷锟斤拷时锟斤拷

/* USER CODE END Private defines */

void MX_QUADSPI_Init(void);

/* USER CODE BEGIN Prototypes */

int8_t	QSPI_W25Qxx_Init(void);						// 初始化
int8_t 	QSPI_W25Qxx_Reset(void);					// 复位
uint32_t QSPI_W25Qxx_ReadID(void);					// 读取ID
int8_t 	QSPI_W25Qxx_MemoryMappedMode(void);		// 内存映射

// 按地址擦除
int8_t 	QSPI_W25Qxx_SectorErase(uint32_t SectorAddress);
// 擦除32K
int8_t 	QSPI_W25Qxx_BlockErase_32K (uint32_t SectorAddress);
// 参数64K
int8_t 	QSPI_W25Qxx_BlockErase_64K (uint32_t SectorAddress);
// 全片擦除
int8_t 	QSPI_W25Qxx_ChipErase (void);

int8_t	QSPI_W25Qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);   // 按页写入数据
int8_t	QSPI_W25Qxx_WriteBuffer(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);     // 写入数据
int8_t 	QSPI_W25Qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);    // 读取数据
int8_t 	QSPI_W25Qxx_ReadBuffer_DMA(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __QUADSPI_H__ */

