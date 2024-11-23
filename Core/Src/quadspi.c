/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    quadspi.c
  * @brief   This file provides code for the configuration
  *          of the QUADSPI instances.
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
/* Includes ------------------------------------------------------------------*/
#include "quadspi.h"

/* USER CODE BEGIN 0 */

// QSPI接收状态标志，该值1时表示接收结束
__IO uint8_t QSPI_RX_Status = 0;
/* USER CODE END 0 */

QSPI_HandleTypeDef hqspi;
MDMA_HandleTypeDef hmdma_quadspi_fifo_th;

/* QUADSPI init function */
void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 1;
  hqspi.Init.FifoThreshold = 32;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
  hqspi.Init.FlashSize = 22;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_3;
  hqspi.Init.FlashID = QSPI_FLASH_ID_1;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}

void HAL_QSPI_MspInit(QSPI_HandleTypeDef* qspiHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspInit 0 */

  /* USER CODE END QUADSPI_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_QSPI;
    PeriphClkInitStruct.QspiClockSelection = RCC_QSPICLKSOURCE_D1HCLK;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* QUADSPI clock enable */
    __HAL_RCC_QSPI_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PB2     ------> QUADSPI_CLK
    PD11     ------> QUADSPI_BK1_IO0
    PD12     ------> QUADSPI_BK1_IO1
    PD13     ------> QUADSPI_BK1_IO3
    PB6     ------> QUADSPI_BK1_NCS
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* QUADSPI MDMA Init */
    /* QUADSPI_FIFO_TH Init */
    hmdma_quadspi_fifo_th.Instance = MDMA_Channel1;
    hmdma_quadspi_fifo_th.Init.Request = MDMA_REQUEST_QUADSPI_FIFO_TH;
    hmdma_quadspi_fifo_th.Init.TransferTriggerMode = MDMA_BUFFER_TRANSFER;
    hmdma_quadspi_fifo_th.Init.Priority = MDMA_PRIORITY_VERY_HIGH;
    hmdma_quadspi_fifo_th.Init.Endianness = MDMA_LITTLE_ENDIANNESS_PRESERVE;
    hmdma_quadspi_fifo_th.Init.SourceInc = MDMA_SRC_INC_BYTE;
    hmdma_quadspi_fifo_th.Init.DestinationInc = MDMA_DEST_INC_DISABLE;
    hmdma_quadspi_fifo_th.Init.SourceDataSize = MDMA_SRC_DATASIZE_BYTE;
    hmdma_quadspi_fifo_th.Init.DestDataSize = MDMA_DEST_DATASIZE_BYTE;
    hmdma_quadspi_fifo_th.Init.DataAlignment = MDMA_DATAALIGN_PACKENABLE;
    hmdma_quadspi_fifo_th.Init.BufferTransferLength = 128;
    hmdma_quadspi_fifo_th.Init.SourceBurst = MDMA_SOURCE_BURST_SINGLE;
    hmdma_quadspi_fifo_th.Init.DestBurst = MDMA_DEST_BURST_SINGLE;
    hmdma_quadspi_fifo_th.Init.SourceBlockAddressOffset = 0;
    hmdma_quadspi_fifo_th.Init.DestBlockAddressOffset = 0;
    if (HAL_MDMA_Init(&hmdma_quadspi_fifo_th) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_MDMA_ConfigPostRequestMask(&hmdma_quadspi_fifo_th, 0, 0) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(qspiHandle,hmdma,hmdma_quadspi_fifo_th);

    /* QUADSPI interrupt Init */
    HAL_NVIC_SetPriority(QUADSPI_IRQn, 14, 0);
    HAL_NVIC_EnableIRQ(QUADSPI_IRQn);
  /* USER CODE BEGIN QUADSPI_MspInit 1 */

  /* USER CODE END QUADSPI_MspInit 1 */
  }
}

void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* qspiHandle)
{

  if(qspiHandle->Instance==QUADSPI)
  {
  /* USER CODE BEGIN QUADSPI_MspDeInit 0 */

  /* USER CODE END QUADSPI_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_QSPI_CLK_DISABLE();

    /**QUADSPI GPIO Configuration
    PE2     ------> QUADSPI_BK1_IO2
    PB2     ------> QUADSPI_CLK
    PD11     ------> QUADSPI_BK1_IO0
    PD12     ------> QUADSPI_BK1_IO1
    PD13     ------> QUADSPI_BK1_IO3
    PB6     ------> QUADSPI_BK1_NCS
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2|GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13);

    /* QUADSPI MDMA DeInit */
    HAL_MDMA_DeInit(qspiHandle->hmdma);

    /* QUADSPI interrupt Deinit */
    HAL_NVIC_DisableIRQ(QUADSPI_IRQn);
  /* USER CODE BEGIN QUADSPI_MspDeInit 1 */

  /* USER CODE END QUADSPI_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

int8_t QSPI_W25Qxx_Init(void){
    uint32_t	Device_ID;	// 器件ID
    QSPI_W25Qxx_Reset();							// 复位器件
    Device_ID = QSPI_W25Qxx_ReadID(); 		// 读取器件ID
    if( Device_ID == W25Qxx_FLASH_ID ) return QSPI_W25Qxx_OK;			// 返回成功标志
    else return W25Qxx_ERROR_INIT;		// 返回错误标志
}

int8_t QSPI_W25Qxx_AutoPollingMemReady(void){
    QSPI_CommandTypeDef     s_command;	   // QSPI传输配置
    QSPI_AutoPollingTypeDef s_config;		// 轮询比较相关配置参数

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;			// 1线指令模式
    s_command.AddressMode       = QSPI_ADDRESS_NONE;					// 无地址模式
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;			//	无交替字节
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;	     	 	// 禁止DDR模式
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;	   	// DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	   	//	每次传输数据都发送指令
    s_command.DataMode          = QSPI_DATA_1_LINE;						// 1线数据模式
    s_command.DummyCycles       = 0;											//	空周期个数
    s_command.Instruction       = W25Qxx_CMD_ReadStatus_REG1;	   // 读状态信息寄存器

    s_config.Match           = 0;   									//	匹配值
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;	      	//	与运算
    s_config.Interval        = 0x10;	                     	//	轮询间隔
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// 自动停止模式
    s_config.StatusBytesSize = 1;	                        	//	状态字节数
    s_config.Mask            = W25Qxx_Status_REG1_BUSY;	   // 对在轮询模式下接收的状态字节进行屏蔽，只比较需要用到的位

    // 发送轮询等待命令
    if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_AUTOPOLLING; // 轮询等待无响应

    return QSPI_W25Qxx_OK; // 通信正常结束
}

int8_t QSPI_W25Qxx_Reset(void){
    QSPI_CommandTypeDef s_command;	// QSPI传输配置

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;   	// 1线指令模式
    s_command.AddressMode 		 = QSPI_ADDRESS_NONE;   			// 无地址模式
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 	// 无交替字节
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     	// 禁止DDR模式
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 	// DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 	// 每次传输数据都发送指令
    s_command.DataMode 			 = QSPI_DATA_NONE;       			// 无数据模式
    s_command.DummyCycles 		 = 0;                     			// 空周期个数
    s_command.Instruction 		 = W25Qxx_CMD_EnableReset;       // 执行复位使能命令

    // 发送复位使能命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_INIT;			// 如果发送失败，返回错误信息

    // 使用自动轮询标志位，等待通信结束
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;	// 轮询等待无响应

    s_command.Instruction  = W25Qxx_CMD_ResetDevice;     // 复位器件命令

    //发送复位器件命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_INIT;		  // 如果发送失败，返回错误信息

    // 使用自动轮询标志位，等待通信结束
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;	// 轮询等待无响应

    return QSPI_W25Qxx_OK;	// 复位成功
}

uint32_t QSPI_W25Qxx_ReadID(void){
    QSPI_CommandTypeDef s_command;	// QSPI传输配置
    uint8_t	QSPI_ReceiveBuff[3];		// 存储QSPI读到的数据
    uint32_t	W25Qxx_ID;					// 器件的ID

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    // 1线指令模式
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;     	 // 24位地址
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  // 无交替字节
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      // 禁止DDR模式
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 // 每次传输数据都发送指令
    s_command.AddressMode		 = QSPI_ADDRESS_NONE;   		 // 无地址模式
    s_command.DataMode			 = QSPI_DATA_1_LINE;       	 // 1线数据模式
    s_command.DummyCycles 		 = 0;                   		 // 空周期个数
    s_command.NbData 				 = 3;                          // 传输数据的长度
    s_command.Instruction 		 = W25Qxx_CMD_JedecID;         // 执行读器件ID命令

    // 发送指令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_INIT;		// 如果发送失败，返回错误信息

    // 接收数据
    if (HAL_QSPI_Receive(&hqspi, QSPI_ReceiveBuff, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;  // 如果接收失败，返回错误信息

    // 将得到的数据组合成ID
    W25Qxx_ID = (QSPI_ReceiveBuff[0] << 16) | (QSPI_ReceiveBuff[1] << 8 ) | QSPI_ReceiveBuff[2];

    return W25Qxx_ID; // 返回ID
}

int8_t QSPI_W25Qxx_MemoryMappedMode(void){
    QSPI_CommandTypeDef      s_command;				 // QSPI传输配置
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;	 // 内存映射访问参数

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1线指令模式
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24位地址
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// 无交替字节
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// 禁止DDR模式
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// 每次传输数据都发送指令
    s_command.AddressMode 		 = QSPI_ADDRESS_4_LINES; 				// 4线地址模式
    s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4线数据模式
    s_command.DummyCycles 		 = 6;                    				// 空周期个数
    s_command.Instruction 		 = W25Qxx_CMD_FastReadQuad_IO; 		// 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令

    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE; // 禁用超时计数器, nCS 保持激活状态
    s_mem_mapped_cfg.TimeOutPeriod     = 0;									 // 超时判断周期

    QSPI_W25Qxx_Reset();		// 复位W25Qxx

    if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)	// 进行配置
        return W25Qxx_ERROR_MemoryMapped; 	// 设置内存映射模式错误


    return QSPI_W25Qxx_OK; // 配置成功
}

int8_t QSPI_W25Qxx_WriteEnable(void){
    QSPI_CommandTypeDef     s_command;	   // QSPI传输配置
    QSPI_AutoPollingTypeDef s_config;		// 轮询比较相关配置参数

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    	// 1线指令模式
    s_command.AddressMode 			= QSPI_ADDRESS_NONE;   		      // 无地址模式
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  	// 无交替字节
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      	// 禁止DDR模式
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  	// DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;		// 每次传输数据都发送指令
    s_command.DataMode 				= QSPI_DATA_NONE;       	      // 无数据模式
    s_command.DummyCycles 			= 0;                   	         // 空周期个数
    s_command.Instruction	 		= W25Qxx_CMD_WriteEnable;      	// 发送写使能命令

    // 发送写使能命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_WriteEnable;

    s_config.Match           = 0x02;  								// 匹配值
    s_config.Mask            = W25Qxx_Status_REG1_WEL;	 		// 读状态寄存器1的第1位（只读），WEL写使能标志位，该标志位为1时，代表可以进行写操作
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;			 	// 与运算
    s_config.StatusBytesSize = 1;									 	// 状态字节数
    s_config.Interval        = 0x10;							 		// 轮询间隔
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// 自动停止模式

    s_command.Instruction    = W25Qxx_CMD_ReadStatus_REG1;	// 读状态信息寄存器
    s_command.DataMode       = QSPI_DATA_1_LINE;					// 1线数据模式
    s_command.NbData         = 1;										// 数据长度

    // 发送轮询等待命令
    if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_AUTOPOLLING; 	// 轮询等待无响应

    return QSPI_W25Qxx_OK;  // 通信正常结束
}

int8_t QSPI_W25Qxx_SectorErase(uint32_t SectorAddress){
    QSPI_CommandTypeDef s_command;	// QSPI传输配置

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    // 1线指令模式
    s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;       // 24位地址模式
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  //	无交替字节
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      // 禁止DDR模式
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	// 每次传输数据都发送指令
    s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;        // 1线地址模式
    s_command.DataMode 				= QSPI_DATA_NONE;             // 无数据
    s_command.DummyCycles 			= 0;                          // 空周期个数
    s_command.Address           	= SectorAddress;              // 要擦除的地址
    s_command.Instruction	 		= W25Qxx_CMD_SectorErase;     // 扇区擦除命令

    // 发送写使能
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;		// 写使能失败

    // 发出擦除命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_Erase;				// 擦除失败

    // 使用自动轮询标志位，等待擦除的结束
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;		// 轮询等待无响应

    return QSPI_W25Qxx_OK; // 擦除成功
}

int8_t QSPI_W25Qxx_BlockErase_32K (uint32_t SectorAddress){
    QSPI_CommandTypeDef s_command;	// QSPI传输配置

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    // 1线指令模式
    s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;       // 24位地址模式
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  //	无交替字节
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      // 禁止DDR模式
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  // DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	// 每次传输数据都发送指令
    s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;        // 1线地址模式
    s_command.DataMode 				= QSPI_DATA_NONE;             // 无数据
    s_command.DummyCycles 			= 0;                          // 空周期个数
    s_command.Address           	= SectorAddress;              // 要擦除的地址
    s_command.Instruction	 		= W25Qxx_CMD_BlockErase_32K;  // 块擦除命令，每次擦除32K字节

    // 发送写使能
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;		// 写使能失败

    // 发出擦除命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_Erase;				// 擦除失败

    // 使用自动轮询标志位，等待擦除的结束
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;		// 轮询等待无响应

    return QSPI_W25Qxx_OK;	// 擦除成功
}

int8_t QSPI_W25Qxx_BlockErase_64K (uint32_t SectorAddress){
    QSPI_CommandTypeDef s_command;	// QSPI传输配置

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;       // 1线指令模式
    s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;          // 24位地址模式
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;     //	无交替字节
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;         // 禁止DDR模式
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;     // DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	     // 每次传输数据都发送指令
    s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;           // 1线地址模式
    s_command.DataMode 				= QSPI_DATA_NONE;                // 无数据
    s_command.DummyCycles 			= 0;                             // 空周期个数
    s_command.Address           	= SectorAddress;                 // 要擦除的地址
    s_command.Instruction	 		= W25Qxx_CMD_BlockErase_64K;     // 块擦除命令，每次擦除64K字节

    // 发送写使能
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;	                        // 写使能失败

    // 发出擦除命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_Erase;			                        // 擦除失败

    // 使用自动轮询标志位，等待擦除的结束
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;	                        // 轮询等待无响应

    return QSPI_W25Qxx_OK;		                                    // 擦除成功
}

int8_t QSPI_W25Qxx_ChipErase (void){
    QSPI_CommandTypeDef s_command;		                            // QSPI传输配置
    QSPI_AutoPollingTypeDef s_config;	                            // 轮询等待配置参数

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;      // 1线指令模式
    s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;         // 24位地址模式
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;    //	无交替字节
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;        // 禁止DDR模式
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;    // DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	    // 每次传输数据都发送指令
    s_command.AddressMode 			= QSPI_ADDRESS_NONE;       	    // 无地址
    s_command.DataMode 				= QSPI_DATA_NONE;               // 无数据
    s_command.DummyCycles 			= 0;                            // 空周期个数
    s_command.Instruction	 		= W25Qxx_CMD_ChipErase;         // 擦除命令，进行整片擦除

    // 发送写使能
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;	// 写使能失败

    // 发出擦除命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_Erase;		 // 擦除失败


    s_config.Match           = 0;   									//	匹配值
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;	      	//	与运算
    s_config.Interval        = 0x10;	                     	//	轮询间隔
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// 自动停止模式
    s_config.StatusBytesSize = 1;	                        	//	状态字节数
    s_config.Mask            = W25Qxx_Status_REG1_BUSY;	   // 对在轮询模式下接收的状态字节进行屏蔽，只比较需要用到的位

    s_command.Instruction    = W25Qxx_CMD_ReadStatus_REG1;	// 读状态信息寄存器
    s_command.DataMode       = QSPI_DATA_1_LINE;					// 1线数据模式
    s_command.NbData         = 1;										// 数据长度

    // W25Q64整片擦除的典型参考时间为20s，最大时间为100s，这里的超时等待值 W25Qxx_ChipErase_TIMEOUT_MAX 为 100S
    if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, W25Qxx_ChipErase_TIMEOUT_MAX) != HAL_OK)
        return W25Qxx_ERROR_AUTOPOLLING;	 // 轮询等待无响应

    return QSPI_W25Qxx_OK;
}

int8_t QSPI_W25Qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite){
    QSPI_CommandTypeDef s_command;	// QSPI传输配置

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1线指令模式
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24位地址
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// 无交替字节
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// 禁止DDR模式
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// 每次传输数据都发送指令
    s_command.AddressMode 		 = QSPI_ADDRESS_1_LINE; 				// 1线地址模式
    s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4线数据模式
    s_command.DummyCycles 		 = 0;                    				// 空周期个数
    s_command.NbData      		 = NumByteToWrite;      			   // 数据长度，最大只能256字节
    s_command.Address     		 = WriteAddr;         					// 要写入 W25Qxx 的地址
    s_command.Instruction 		 = W25Qxx_CMD_QuadInputPageProgram; // 1-1-4模式下(1线指令1线地址4线数据)，页编程指令

    // 写使能
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;	// 写使能失败

    // 写命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// 传输数据错误

    // 开始传输数据
    if (HAL_QSPI_Transmit(&hqspi, pBuffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// 传输数据错误

    // 使用自动轮询标志位，等待写入的结束
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING; // 轮询等待无响应

    return QSPI_W25Qxx_OK;	// 写数据成功
}

int8_t QSPI_W25Qxx_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t Size){
    uint32_t end_addr, current_size, current_addr;
    uint8_t *write_data;  // 要写入的数据

    current_size = W25Qxx_PageSize - (WriteAddr % W25Qxx_PageSize); // 计算当前页还剩余的空间

    if (current_size > Size)	// 判断当前页剩余的空间是否足够写入所有数据
        current_size = Size;		// 如果足够，则直接获取当前长度


    current_addr = WriteAddr;		// 获取要写入的地址
    end_addr = WriteAddr + Size;	// 计算结束地址
    write_data = pBuffer;			// 获取要写入的数据

    do{
        // 按页写入数据
        if(QSPI_W25Qxx_WritePage(write_data, current_addr, current_size) != QSPI_W25Qxx_OK)
            return W25Qxx_ERROR_TRANSMIT;


        else {
            current_addr += current_size;	// 计算下一次要写入的地址
            write_data += current_size;	// 获取下一次要写入的数据存储区地址
            // 计算下一次写数据的长度
            current_size = ((current_addr + W25Qxx_PageSize) > end_addr) ? (end_addr - current_addr) : W25Qxx_PageSize;
        }
    }
    while (current_addr < end_addr) ; // 判断数据是否全部写入完毕

    return QSPI_W25Qxx_OK;	// 写入数据成功
}

int8_t QSPI_W25Qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead) {
    QSPI_CommandTypeDef s_command;	// QSPI传输配置

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1线指令模式
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24位地址
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// 无交替字节
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// 禁止DDR模式
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// 每次传输数据都发送指令
    s_command.AddressMode 		 = QSPI_ADDRESS_4_LINES; 				// 4线地址模式
    s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4线数据模式
    s_command.DummyCycles 		 = 6;                    				// 空周期个数
    s_command.NbData      		 = NumByteToRead;      			   	// 数据长度，最大不能超过flash芯片的大小
    s_command.Address     		 = ReadAddr;         					// 要读取 W25Qxx 的地址
    s_command.Instruction 		 = W25Qxx_CMD_FastReadQuad_IO; 		// 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令

    // 发送读取命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// 传输数据错误

    //	接收数据

    if (HAL_QSPI_Receive(&hqspi, pBuffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// 传输数据错误

    // 使用自动轮询标志位，等待接收的结束
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING; // 轮询等待无响应

    return QSPI_W25Qxx_OK;	// 读取数据成功
}

int8_t QSPI_W25Qxx_ReadBuffer_DMA(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead){
    QSPI_CommandTypeDef s_command;	// QSPI传输配置

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1线指令模式
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24位地址
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// 无交替字节
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// 禁止DDR模式
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDR模式中数据延迟，这里用不到
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// 每次传输数据都发送指令
    s_command.AddressMode 		 = QSPI_ADDRESS_4_LINES; 				// 4线地址模式
    s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4线数据模式
    s_command.DummyCycles 		 = 6;                    				// 空周期个数
    s_command.NbData      		 = NumByteToRead;      			   	// 数据长度，最大不能超过flash芯片的大小
    s_command.Address     		 = ReadAddr;         					// 要读取 W25Qxx 的地址
    s_command.Instruction 		 = W25Qxx_CMD_FastReadQuad_IO; 		// 1-4-4模式下(1线指令4线地址4线数据)，快速读取指令

    // 发送读取命令
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// 传输数据错误


    //	接收数据
    if (HAL_QSPI_Receive_DMA(&hqspi, pBuffer) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// 传输数据错误


    while(QSPI_RX_Status == 0); // 等待传输完成
    QSPI_RX_Status = 0;			 // 清零标志位

    return QSPI_W25Qxx_OK;		// 读取数据成功
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi){
    QSPI_RX_Status = 1;  // 当进入此中断函数时，说明QSPI接收完成，将标志变量置1
}

/* USER CODE END 1 */
