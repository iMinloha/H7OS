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

#define QSPI_W25Qxx_OK           		0		// W25Qxxͨ������
#define W25Qxx_ERROR_INIT         		-1		// ��ʼ������
#define W25Qxx_ERROR_WriteEnable       -2		// дʹ�ܴ���
#define W25Qxx_ERROR_AUTOPOLLING       -3		// ��ѯ�ȴ���������Ӧ
#define W25Qxx_ERROR_Erase         		-4		// ��������
#define W25Qxx_ERROR_TRANSMIT         	-5		// �������?
#define W25Qxx_ERROR_MemoryMapped		-6    // �ڴ�ӳ��ģʽ����

#define W25Qxx_CMD_EnableReset  		0x66		// ʹ�ܸ�λ
#define W25Qxx_CMD_ResetDevice   	0x99		// ��λ����
#define W25Qxx_CMD_JedecID 			0x9F		// JEDEC ID
#define W25Qxx_CMD_WriteEnable		0X06		// дʹ��

#define W25Qxx_CMD_SectorErase 		0x20		// ����������4K�ֽڣ� �ο�����ʱ�� 45ms
#define W25Qxx_CMD_BlockErase_32K 	0x52		// �������?  32K�ֽڣ��ο�����ʱ�� 120ms
#define W25Qxx_CMD_BlockErase_64K 	0xD8		// �������?  64K�ֽڣ��ο�����ʱ�� 150ms
#define W25Qxx_CMD_ChipErase 			0xC7		// ��Ƭ�������ο�����ʱ�� 20S

#define W25Qxx_CMD_QuadInputPageProgram  	0x32  		// 1-1-4ģʽ��(1��ָ��1�ߵ�ַ4������)��ҳ���ָ��ο�д��ʱ��? 0.4ms
#define W25Qxx_CMD_FastReadQuad_IO       	0xEB  		// 1-4-4ģʽ��(1��ָ��4�ߵ�ַ4������)�����ٶ�ȡָ��

#define W25Qxx_CMD_ReadStatus_REG1			0X05			// ��״̬�Ĵ���1
#define W25Qxx_Status_REG1_BUSY  			0x01			// ��״̬�Ĵ���1�ĵ�0λ��ֻ������Busy��־λ�������ڲ���/д������/д����ʱ�ᱻ��1
#define W25Qxx_Status_REG1_WEL  				0x02			// ��״̬�Ĵ���1�ĵ�1λ��ֻ������WELдʹ�ܱ�־λ���ñ�־λΪ1ʱ���������Խ���д����

#define W25Qxx_PageSize       				256			// ҳ��С��256�ֽ�
#define W25Qxx_FlashSize       				0x800000		// W25Q64��С��8M�ֽ�
#define W25Qxx_FLASH_ID           			0Xef4017    // W25Q64 JEDEC ID
#define W25Qxx_ChipErase_TIMEOUT_MAX		100000U		// ��ʱ�ȴ�ʱ�䣬W25Q64��Ƭ�����������ʱ����?100S
#define W25Qxx_Mem_Addr							0x90000000 	// �ڴ�ӳ��ģʽ�ĵ�ַ


#define QUADSPI_CLK_PIN							GPIO_PIN_2								// QUADSPI_CLK ����
#define	QUADSPI_CLK_PORT							GPIOB										// QUADSPI_CLK ���Ŷ˿�
#define	QUADSPI_CLK_AF								GPIO_AF9_QUADSPI						// QUADSPI_CLK IO�ڸ���
#define GPIO_QUADSPI_CLK_ENABLE      			__HAL_RCC_GPIOB_CLK_ENABLE()	 	// QUADSPI_CLK ����ʱ��

#define QUADSPI_BK1_NCS_PIN						GPIO_PIN_6								// QUADSPI_BK1_NCS ����
#define	QUADSPI_BK1_NCS_PORT						GPIOB										// QUADSPI_BK1_NCS ���Ŷ˿�
#define	QUADSPI_BK1_NCS_AF						GPIO_AF10_QUADSPI						// QUADSPI_BK1_NCS IO�ڸ���
#define GPIO_QUADSPI_BK1_NCS_ENABLE        	__HAL_RCC_GPIOB_CLK_ENABLE()	 	// QUADSPI_BK1_NCS ����ʱ��

#define QUADSPI_BK1_IO0_PIN						GPIO_PIN_11								// QUADSPI_BK1_IO0 ����
#define	QUADSPI_BK1_IO0_PORT						GPIOD										// QUADSPI_BK1_IO0 ���Ŷ˿�
#define	QUADSPI_BK1_IO0_AF						GPIO_AF9_QUADSPI						// QUADSPI_BK1_IO0 IO�ڸ���
#define GPIO_QUADSPI_BK1_IO0_ENABLE        	__HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO0 ����ʱ��

#define QUADSPI_BK1_IO1_PIN						GPIO_PIN_12								// QUADSPI_BK1_IO1 ����
#define	QUADSPI_BK1_IO1_PORT						GPIOD										// QUADSPI_BK1_IO1 ���Ŷ˿�
#define	QUADSPI_BK1_IO1_AF						GPIO_AF9_QUADSPI						// QUADSPI_BK1_IO1 IO�ڸ���
#define GPIO_QUADSPI_BK1_IO1_ENABLE        	__HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO1 ����ʱ��

#define QUADSPI_BK1_IO2_PIN						GPIO_PIN_2								// QUADSPI_BK1_IO2 ����
#define	QUADSPI_BK1_IO2_PORT						GPIOE										// QUADSPI_BK1_IO2 ���Ŷ˿�
#define	QUADSPI_BK1_IO2_AF						GPIO_AF9_QUADSPI						// QUADSPI_BK1_IO2 IO�ڸ���
#define GPIO_QUADSPI_BK1_IO2_ENABLE        	__HAL_RCC_GPIOE_CLK_ENABLE()	 	// QUADSPI_BK1_IO2 ����ʱ��

#define QUADSPI_BK1_IO3_PIN						GPIO_PIN_13								// QUADSPI_BK1_IO3 ����
#define	QUADSPI_BK1_IO3_PORT						GPIOD										// QUADSPI_BK1_IO3 ���Ŷ˿�
#define	QUADSPI_BK1_IO3_AF						GPIO_AF9_QUADSPI						// QUADSPI_BK1_IO3 IO�ڸ���
#define GPIO_QUADSPI_BK1_IO3_ENABLE      	__HAL_RCC_GPIOD_CLK_ENABLE()	 	// QUADSPI_BK1_IO3 ����ʱ��

/* USER CODE END Private defines */

void MX_QUADSPI_Init(void);

/* USER CODE BEGIN Prototypes */

int8_t	QSPI_W25Qxx_Init(void);						// ��ʼ��
int8_t 	QSPI_W25Qxx_Reset(void);					// ��λ
uint32_t QSPI_W25Qxx_ReadID(void);					// ��ȡID
int8_t 	QSPI_W25Qxx_MemoryMappedMode(void);		// �ڴ�ӳ��

// ����ַ����
int8_t 	QSPI_W25Qxx_SectorErase(uint32_t SectorAddress);
// ����32K
int8_t 	QSPI_W25Qxx_BlockErase_32K (uint32_t SectorAddress);
// ����64K
int8_t 	QSPI_W25Qxx_BlockErase_64K (uint32_t SectorAddress);
// ȫƬ����
int8_t 	QSPI_W25Qxx_ChipErase (void);

int8_t	QSPI_W25Qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);   // ��ҳд������
int8_t	QSPI_W25Qxx_WriteBuffer(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);     // д������
int8_t 	QSPI_W25Qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);    // ��ȡ����

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __QUADSPI_H__ */

