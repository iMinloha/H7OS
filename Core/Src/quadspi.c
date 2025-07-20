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

// QSPI����״̬��־����ֵ1ʱ��ʾ���ս���
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
    uint32_t	Device_ID;	// ����ID
    QSPI_W25Qxx_Reset();							// ��λ����
    Device_ID = QSPI_W25Qxx_ReadID(); 		// ��ȡ����ID
    if( Device_ID == W25Qxx_FLASH_ID ) return QSPI_W25Qxx_OK;			// ���سɹ���־
    else return W25Qxx_ERROR_INIT;		// ���ش����־
}

int8_t QSPI_W25Qxx_AutoPollingMemReady(void){
    QSPI_CommandTypeDef     s_command;	   // QSPI��������
    QSPI_AutoPollingTypeDef s_config;		// ��ѯ�Ƚ�������ò���

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;			// 1��ָ��ģʽ
    s_command.AddressMode       = QSPI_ADDRESS_NONE;					// �޵�ַģʽ
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;			//	�޽����ֽ�
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;	     	 	// ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;	   	// DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	   	//	ÿ�δ������ݶ�����ָ��
    s_command.DataMode          = QSPI_DATA_1_LINE;						// 1������ģʽ
    s_command.DummyCycles       = 0;											//	�����ڸ���
    s_command.Instruction       = W25Qxx_CMD_ReadStatus_REG1;	   // ��״̬��Ϣ�Ĵ���

    s_config.Match           = 0;   									//	ƥ��ֵ
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;	      	//	������
    s_config.Interval        = 0x10;	                     	//	��ѯ���
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// �Զ�ֹͣģʽ
    s_config.StatusBytesSize = 1;	                        	//	״̬�ֽ���
    s_config.Mask            = W25Qxx_Status_REG1_BUSY;	   // ������ѯģʽ�½��յ�״̬�ֽڽ������Σ�ֻ�Ƚ���Ҫ�õ���λ

    // ������ѯ�ȴ�����
    if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_AUTOPOLLING; // ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK; // ͨ����������
}

int8_t QSPI_W25Qxx_Reset(void){
    QSPI_CommandTypeDef s_command;	// QSPI��������

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;   	// 1��ָ��ģʽ
    s_command.AddressMode 		 = QSPI_ADDRESS_NONE;   			// �޵�ַģʽ
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 	// �޽����ֽ�
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     	// ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 	// DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 	// ÿ�δ������ݶ�����ָ��
    s_command.DataMode 			 = QSPI_DATA_NONE;       			// ������ģʽ
    s_command.DummyCycles 		 = 0;                     			// �����ڸ���
    s_command.Instruction 		 = W25Qxx_CMD_EnableReset;       // ִ�и�λʹ������

    // ���͸�λʹ������
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_INIT;			// �������ʧ�ܣ����ش�����Ϣ

    // ʹ���Զ���ѯ��־λ���ȴ�ͨ�Ž���
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;	// ��ѯ�ȴ�����Ӧ

    s_command.Instruction  = W25Qxx_CMD_ResetDevice;     // ��λ��������

    //���͸�λ��������
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_INIT;		  // �������ʧ�ܣ����ش�����Ϣ

    // ʹ���Զ���ѯ��־λ���ȴ�ͨ�Ž���
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;	// ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK;	// ��λ�ɹ�
}

uint32_t QSPI_W25Qxx_ReadID(void){
    QSPI_CommandTypeDef s_command;	// QSPI��������
    uint8_t	QSPI_ReceiveBuff[3];		// �洢QSPI����������
    uint32_t	W25Qxx_ID;					// ������ID

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    // 1��ָ��ģʽ
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;     	 // 24λ��ַ
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  // �޽����ֽ�
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;      // ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;  // DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;	 // ÿ�δ������ݶ�����ָ��
    s_command.AddressMode		 = QSPI_ADDRESS_NONE;   		 // �޵�ַģʽ
    s_command.DataMode			 = QSPI_DATA_1_LINE;       	 // 1������ģʽ
    s_command.DummyCycles 		 = 0;                   		 // �����ڸ���
    s_command.NbData 				 = 3;                          // �������ݵĳ���
    s_command.Instruction 		 = W25Qxx_CMD_JedecID;         // ִ�ж�����ID����

    // ����ָ��
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_INIT;		// �������ʧ�ܣ����ش�����Ϣ

    // ��������
    if (HAL_QSPI_Receive(&hqspi, QSPI_ReceiveBuff, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;  // �������ʧ�ܣ����ش�����Ϣ

    // ���õ���������ϳ�ID
    W25Qxx_ID = (QSPI_ReceiveBuff[0] << 16) | (QSPI_ReceiveBuff[1] << 8 ) | QSPI_ReceiveBuff[2];

    return W25Qxx_ID; // ����ID
}

int8_t QSPI_W25Qxx_MemoryMappedMode(void){
    QSPI_CommandTypeDef      s_command;				 // QSPI��������
    QSPI_MemoryMappedTypeDef s_mem_mapped_cfg;	 // �ڴ�ӳ����ʲ���

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1��ָ��ģʽ
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24λ��ַ
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// �޽����ֽ�
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// ÿ�δ������ݶ�����ָ��
    s_command.AddressMode 		 = QSPI_ADDRESS_4_LINES; 				// 4�ߵ�ַģʽ
    s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4������ģʽ
    s_command.DummyCycles 		 = 6;                    				// �����ڸ���
    s_command.Instruction 		 = W25Qxx_CMD_FastReadQuad_IO; 		// 1-4-4ģʽ��(1��ָ��4�ߵ�ַ4������)�����ٶ�ȡָ��

    s_mem_mapped_cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE; // ���ó�ʱ������, nCS ���ּ���״̬
    s_mem_mapped_cfg.TimeOutPeriod     = 0;									 // ��ʱ�ж�����

    QSPI_W25Qxx_Reset();		// ��λW25Qxx

    if (HAL_QSPI_MemoryMapped(&hqspi, &s_command, &s_mem_mapped_cfg) != HAL_OK)	// ��������
        return W25Qxx_ERROR_MemoryMapped; 	// �����ڴ�ӳ��ģʽ����


    return QSPI_W25Qxx_OK; // ���óɹ�
}

int8_t QSPI_W25Qxx_WriteEnable(void){
    QSPI_CommandTypeDef     s_command;	   // QSPI��������
    QSPI_AutoPollingTypeDef s_config;		// ��ѯ�Ƚ�������ò���

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    	// 1��ָ��ģʽ
    s_command.AddressMode 			= QSPI_ADDRESS_NONE;   		      // �޵�ַģʽ
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  	// �޽����ֽ�
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      	// ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  	// DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;		// ÿ�δ������ݶ�����ָ��
    s_command.DataMode 				= QSPI_DATA_NONE;       	      // ������ģʽ
    s_command.DummyCycles 			= 0;                   	         // �����ڸ���
    s_command.Instruction	 		= W25Qxx_CMD_WriteEnable;      	// ����дʹ������

    // ����дʹ������
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_WriteEnable;

    s_config.Match           = 0x02;  								// ƥ��ֵ
    s_config.Mask            = W25Qxx_Status_REG1_WEL;	 		// ��״̬�Ĵ���1�ĵ�1λ��ֻ������WELдʹ�ܱ�־λ���ñ�־λΪ1ʱ��������Խ���д����
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;			 	// ������
    s_config.StatusBytesSize = 1;									 	// ״̬�ֽ���
    s_config.Interval        = 0x10;							 		// ��ѯ���
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// �Զ�ֹͣģʽ

    s_command.Instruction    = W25Qxx_CMD_ReadStatus_REG1;	// ��״̬��Ϣ�Ĵ���
    s_command.DataMode       = QSPI_DATA_1_LINE;					// 1������ģʽ
    s_command.NbData         = 1;										// ���ݳ���

    // ������ѯ�ȴ�����
    if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_AUTOPOLLING; 	// ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK;  // ͨ����������
}

int8_t QSPI_W25Qxx_SectorErase(uint32_t SectorAddress){
    QSPI_CommandTypeDef s_command;	// QSPI��������

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    // 1��ָ��ģʽ
    s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;       // 24λ��ַģʽ
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  //	�޽����ֽ�
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      // ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  // DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	// ÿ�δ������ݶ�����ָ��
    s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;        // 1�ߵ�ַģʽ
    s_command.DataMode 				= QSPI_DATA_NONE;             // ������
    s_command.DummyCycles 			= 0;                          // �����ڸ���
    s_command.Address           	= SectorAddress;              // Ҫ�����ĵ�ַ
    s_command.Instruction	 		= W25Qxx_CMD_SectorErase;     // ������������

    // ����дʹ��
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;		// дʹ��ʧ��

    // ������������
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_Erase;				// ����ʧ��

    // ʹ���Զ���ѯ��־λ���ȴ������Ľ���
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;		// ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK; // �����ɹ�
}

int8_t QSPI_W25Qxx_BlockErase_32K (uint32_t SectorAddress){
    QSPI_CommandTypeDef s_command;	// QSPI��������

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;    // 1��ָ��ģʽ
    s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;       // 24λ��ַģʽ
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;  //	�޽����ֽ�
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;      // ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;  // DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	// ÿ�δ������ݶ�����ָ��
    s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;        // 1�ߵ�ַģʽ
    s_command.DataMode 				= QSPI_DATA_NONE;             // ������
    s_command.DummyCycles 			= 0;                          // �����ڸ���
    s_command.Address           	= SectorAddress;              // Ҫ�����ĵ�ַ
    s_command.Instruction	 		= W25Qxx_CMD_BlockErase_32K;  // ��������ÿ�β���32K�ֽ�

    // ����дʹ��
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;		// дʹ��ʧ��

    // ������������
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_Erase;				// ����ʧ��

    // ʹ���Զ���ѯ��־λ���ȴ������Ľ���
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;		// ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK;	// �����ɹ�
}

int8_t QSPI_W25Qxx_BlockErase_64K (uint32_t SectorAddress){
    QSPI_CommandTypeDef s_command;	// QSPI��������

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;       // 1��ָ��ģʽ
    s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;          // 24λ��ַģʽ
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;     //	�޽����ֽ�
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;         // ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;     // DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	     // ÿ�δ������ݶ�����ָ��
    s_command.AddressMode 			= QSPI_ADDRESS_1_LINE;           // 1�ߵ�ַģʽ
    s_command.DataMode 				= QSPI_DATA_NONE;                // ������
    s_command.DummyCycles 			= 0;                             // �����ڸ���
    s_command.Address           	= SectorAddress;                 // Ҫ�����ĵ�ַ
    s_command.Instruction	 		= W25Qxx_CMD_BlockErase_64K;     // ��������ÿ�β���64K�ֽ�

    // ����дʹ��
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;	                        // дʹ��ʧ��

    // ������������
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_Erase;			                        // ����ʧ��

    // ʹ���Զ���ѯ��־λ���ȴ������Ľ���
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING;	                        // ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK;		                                    // �����ɹ�
}

int8_t QSPI_W25Qxx_ChipErase (void){
    QSPI_CommandTypeDef s_command;		                            // QSPI��������
    QSPI_AutoPollingTypeDef s_config;	                            // ��ѯ�ȴ����ò���

    s_command.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;      // 1��ָ��ģʽ
    s_command.AddressSize       	= QSPI_ADDRESS_24_BITS;         // 24λ��ַģʽ
    s_command.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;    //	�޽����ֽ�
    s_command.DdrMode           	= QSPI_DDR_MODE_DISABLE;        // ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;    // DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;	    // ÿ�δ������ݶ�����ָ��
    s_command.AddressMode 			= QSPI_ADDRESS_NONE;       	    // �޵�ַ
    s_command.DataMode 				= QSPI_DATA_NONE;               // ������
    s_command.DummyCycles 			= 0;                            // �����ڸ���
    s_command.Instruction	 		= W25Qxx_CMD_ChipErase;         // �������������Ƭ����

    // ����дʹ��
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;	// дʹ��ʧ��

    // ������������
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_Erase;		 // ����ʧ��


    s_config.Match           = 0;   									//	ƥ��ֵ
    s_config.MatchMode       = QSPI_MATCH_MODE_AND;	      	//	������
    s_config.Interval        = 0x10;	                     	//	��ѯ���
    s_config.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;	// �Զ�ֹͣģʽ
    s_config.StatusBytesSize = 1;	                        	//	״̬�ֽ���
    s_config.Mask            = W25Qxx_Status_REG1_BUSY;	   // ������ѯģʽ�½��յ�״̬�ֽڽ������Σ�ֻ�Ƚ���Ҫ�õ���λ

    s_command.Instruction    = W25Qxx_CMD_ReadStatus_REG1;	// ��״̬��Ϣ�Ĵ���
    s_command.DataMode       = QSPI_DATA_1_LINE;					// 1������ģʽ
    s_command.NbData         = 1;										// ���ݳ���

    // W25Q64��Ƭ�����ĵ��Ͳο�ʱ��Ϊ20s�����ʱ��Ϊ100s������ĳ�ʱ�ȴ�ֵ W25Qxx_ChipErase_TIMEOUT_MAX Ϊ 100S
    if (HAL_QSPI_AutoPolling(&hqspi, &s_command, &s_config, W25Qxx_ChipErase_TIMEOUT_MAX) != HAL_OK)
        return W25Qxx_ERROR_AUTOPOLLING;	 // ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK;
}

int8_t QSPI_W25Qxx_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite){
    QSPI_CommandTypeDef s_command;	// QSPI��������

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1��ָ��ģʽ
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24λ��ַ
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// �޽����ֽ�
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// ÿ�δ������ݶ�����ָ��
    s_command.AddressMode 		 = QSPI_ADDRESS_1_LINE; 				// 1�ߵ�ַģʽ
    s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4������ģʽ
    s_command.DummyCycles 		 = 0;                    				// �����ڸ���
    s_command.NbData      		 = NumByteToWrite;      			   // ���ݳ��ȣ����ֻ��256�ֽ�
    s_command.Address     		 = WriteAddr;         					// Ҫд�� W25Qxx �ĵ�ַ
    s_command.Instruction 		 = W25Qxx_CMD_QuadInputPageProgram; // 1-1-4ģʽ��(1��ָ��1�ߵ�ַ4������)��ҳ���ָ��

    // дʹ��
    if (QSPI_W25Qxx_WriteEnable() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_WriteEnable;	// дʹ��ʧ��

    // д����
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// �������ݴ���

    // ��ʼ��������
    if (HAL_QSPI_Transmit(&hqspi, pBuffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// �������ݴ���

    // ʹ���Զ���ѯ��־λ���ȴ�д��Ľ���
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING; // ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK;	// д���ݳɹ�
}

int8_t QSPI_W25Qxx_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint32_t Size){
    uint32_t end_addr, current_size, current_addr;
    uint8_t *write_data;  // Ҫд�������

    current_size = W25Qxx_PageSize - (WriteAddr % W25Qxx_PageSize); // ���㵱ǰҳ��ʣ��Ŀռ�

    if (current_size > Size)	// �жϵ�ǰҳʣ��Ŀռ��Ƿ��㹻д����������
        current_size = Size;		// ����㹻����ֱ�ӻ�ȡ��ǰ����


    current_addr = WriteAddr;		// ��ȡҪд��ĵ�ַ
    end_addr = WriteAddr + Size;	// ���������ַ
    write_data = pBuffer;			// ��ȡҪд�������

    do{
        // ��ҳд������
        if(QSPI_W25Qxx_WritePage(write_data, current_addr, current_size) != QSPI_W25Qxx_OK)
            return W25Qxx_ERROR_TRANSMIT;


        else {
            current_addr += current_size;	// ������һ��Ҫд��ĵ�ַ
            write_data += current_size;	// ��ȡ��һ��Ҫд������ݴ洢����ַ
            // ������һ��д���ݵĳ���
            current_size = ((current_addr + W25Qxx_PageSize) > end_addr) ? (end_addr - current_addr) : W25Qxx_PageSize;
        }
    }
    while (current_addr < end_addr) ; // �ж������Ƿ�ȫ��д�����

    return QSPI_W25Qxx_OK;	// д�����ݳɹ�
}

int8_t QSPI_W25Qxx_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead) {
    QSPI_CommandTypeDef s_command;	// QSPI��������

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1��ָ��ģʽ
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24λ��ַ
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// �޽����ֽ�
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// ÿ�δ������ݶ�����ָ��
    s_command.AddressMode 		 = QSPI_ADDRESS_4_LINES; 				// 4�ߵ�ַģʽ
    s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4������ģʽ
    s_command.DummyCycles 		 = 6;                    				// �����ڸ���
    s_command.NbData      		 = NumByteToRead;      			   	// ���ݳ��ȣ�����ܳ���flashоƬ�Ĵ�С
    s_command.Address     		 = ReadAddr;         					// Ҫ��ȡ W25Qxx �ĵ�ַ
    s_command.Instruction 		 = W25Qxx_CMD_FastReadQuad_IO; 		// 1-4-4ģʽ��(1��ָ��4�ߵ�ַ4������)�����ٶ�ȡָ��

    // ���Ͷ�ȡ����
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// �������ݴ���

    //	��������

    if (HAL_QSPI_Receive(&hqspi, pBuffer, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// �������ݴ���

    // ʹ���Զ���ѯ��־λ���ȴ����յĽ���
    if (QSPI_W25Qxx_AutoPollingMemReady() != QSPI_W25Qxx_OK)
        return W25Qxx_ERROR_AUTOPOLLING; // ��ѯ�ȴ�����Ӧ

    return QSPI_W25Qxx_OK;	// ��ȡ���ݳɹ�
}

int8_t QSPI_W25Qxx_ReadBuffer_DMA(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead){
    QSPI_CommandTypeDef s_command;	// QSPI��������

    s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;    		// 1��ָ��ģʽ
    s_command.AddressSize       = QSPI_ADDRESS_24_BITS;            // 24λ��ַ
    s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;  		// �޽����ֽ�
    s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;     		// ��ֹDDRģʽ
    s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY; 		// DDRģʽ�������ӳ٣������ò���
    s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;			// ÿ�δ������ݶ�����ָ��
    s_command.AddressMode 		 = QSPI_ADDRESS_4_LINES; 				// 4�ߵ�ַģʽ
    s_command.DataMode    		 = QSPI_DATA_4_LINES;    				// 4������ģʽ
    s_command.DummyCycles 		 = 6;                    				// �����ڸ���
    s_command.NbData      		 = NumByteToRead;      			   	// ���ݳ��ȣ�����ܳ���flashоƬ�Ĵ�С
    s_command.Address     		 = ReadAddr;         					// Ҫ��ȡ W25Qxx �ĵ�ַ
    s_command.Instruction 		 = W25Qxx_CMD_FastReadQuad_IO; 		// 1-4-4ģʽ��(1��ָ��4�ߵ�ַ4������)�����ٶ�ȡָ��

    // ���Ͷ�ȡ����
    if (HAL_QSPI_Command(&hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// �������ݴ���


    //	��������
    if (HAL_QSPI_Receive_DMA(&hqspi, pBuffer) != HAL_OK)
        return W25Qxx_ERROR_TRANSMIT;		// �������ݴ���


    while(QSPI_RX_Status == 0); // �ȴ��������
    QSPI_RX_Status = 0;			 // �����־λ

    return QSPI_W25Qxx_OK;		// ��ȡ���ݳɹ�
}

void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi){
    QSPI_RX_Status = 1;  // ��������жϺ���ʱ��˵��QSPI������ɣ�����־������1
}

/* USER CODE END 1 */
