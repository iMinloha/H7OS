/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ltdc.c
  * @brief   This file provides code for the configuration
  *          of the LTDC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "ltdc.h"

/* USER CODE BEGIN 0 */
#include <stdio.h>

#include "dma2d.h"
#include "ff.h"

extern DMA2D_HandleTypeDef hdma2d;
/* USER CODE END 0 */

LTDC_HandleTypeDef hltdc;

/* LTDC init function */
void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 0;
  hltdc.Init.VerticalSync = 0;
  hltdc.Init.AccumulatedHBP = 43;
  hltdc.Init.AccumulatedVBP = 12;
  hltdc.Init.AccumulatedActiveW = 523;
  hltdc.Init.AccumulatedActiveH = 284;
  hltdc.Init.TotalWidth = 531;
  hltdc.Init.TotalHeigh = 292;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 480;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 272;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0xC0000000;
  pLayerCfg.ImageWidth = 0;
  pLayerCfg.ImageHeight = 0;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */
	LCD_DisplayDirection(Direction_H);
	LCD_SetFont(&Font24);
	LCD_ShowNumMode(Fill_Space);
	LCD_SetLayer(0);
	LCD_SetBackColor(LCD_BLACK);
	LCD_SetColor(LCD_WHITE);
	LCD_Clear();

	LCD_Backlight_ON;
  /* USER CODE END LTDC_Init 2 */

}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef* ltdcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(ltdcHandle->Instance==LTDC)
  {
  /* USER CODE BEGIN LTDC_MspInit 0 */

  /* USER CODE END LTDC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
    PeriphClkInitStruct.PLL3.PLL3M = 25;
    PeriphClkInitStruct.PLL3.PLL3N = 200;
    PeriphClkInitStruct.PLL3.PLL3P = 2;
    PeriphClkInitStruct.PLL3.PLL3Q = 2;
    PeriphClkInitStruct.PLL3.PLL3R = 20;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* LTDC clock enable */
    __HAL_RCC_LTDC_CLK_ENABLE();

    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**LTDC GPIO Configuration
    PE5     ------> LTDC_G0
    PE6     ------> LTDC_G1
    PI9     ------> LTDC_VSYNC
    PI10     ------> LTDC_HSYNC
    PF10     ------> LTDC_DE
    PA2     ------> LTDC_R1
    PH8     ------> LTDC_R2
    PH9     ------> LTDC_R3
    PH10     ------> LTDC_R4
    PH11     ------> LTDC_R5
    PH12     ------> LTDC_R6
    PG6     ------> LTDC_R7
    PG7     ------> LTDC_CLK
    PA8     ------> LTDC_B3
    PH13     ------> LTDC_G2
    PH14     ------> LTDC_G3
    PH15     ------> LTDC_G4
    PI0     ------> LTDC_G5
    PI1     ------> LTDC_G6
    PI2     ------> LTDC_G7
    PD6     ------> LTDC_B2
    PG12     ------> LTDC_B1
    PG13     ------> LTDC_R0
    PG14     ------> LTDC_B0
    PI4     ------> LTDC_B4
    PI5     ------> LTDC_B5
    PI6     ------> LTDC_B6
    PI7     ------> LTDC_B7
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF13_LTDC;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* USER CODE BEGIN LTDC_MspInit 1 */
  	LCD_Backlight_OFF;
  /* USER CODE END LTDC_MspInit 1 */
  }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef* ltdcHandle)
{

  if(ltdcHandle->Instance==LTDC)
  {
  /* USER CODE BEGIN LTDC_MspDeInit 0 */

  /* USER CODE END LTDC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LTDC_CLK_DISABLE();

    /**LTDC GPIO Configuration
    PE5     ------> LTDC_G0
    PE6     ------> LTDC_G1
    PI9     ------> LTDC_VSYNC
    PI10     ------> LTDC_HSYNC
    PF10     ------> LTDC_DE
    PA2     ------> LTDC_R1
    PH8     ------> LTDC_R2
    PH9     ------> LTDC_R3
    PH10     ------> LTDC_R4
    PH11     ------> LTDC_R5
    PH12     ------> LTDC_R6
    PG6     ------> LTDC_R7
    PG7     ------> LTDC_CLK
    PA8     ------> LTDC_B3
    PH13     ------> LTDC_G2
    PH14     ------> LTDC_G3
    PH15     ------> LTDC_G4
    PI0     ------> LTDC_G5
    PI1     ------> LTDC_G6
    PI2     ------> LTDC_G7
    PD6     ------> LTDC_B2
    PG12     ------> LTDC_B1
    PG13     ------> LTDC_R0
    PG14     ------> LTDC_B0
    PI4     ------> LTDC_B4
    PI5     ------> LTDC_B5
    PI6     ------> LTDC_B6
    PI7     ------> LTDC_B7
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_5|GPIO_PIN_6);

    HAL_GPIO_DeInit(GPIOI, GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_0|GPIO_PIN_1
                          |GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_10);

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_8);

    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_12|GPIO_PIN_13
                          |GPIO_PIN_14);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_6);

  /* USER CODE BEGIN LTDC_MspDeInit 1 */
  /* USER CODE END LTDC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
struct {
	uint32_t Color;
	uint32_t BackColor;
	uint32_t ColorMode;
	uint32_t LayerMemoryAdd;
	uint8_t  Layer;
	uint8_t  Direction;
	uint8_t  BytesPerPixel;
	uint8_t  ShowNum_Mode;
} LCD;

static pFONT *LCD_Fonts;
static pChineseFont 	*LCD_CHFonts;


void LCD_SetLayer(uint8_t layer) {
#if LCD_NUM_LAYERS == 2
	if (layer == 0) {
		LCD.LayerMemoryAdd = LCD_MemoryAdd;
		LCD.ColorMode      = ColorMode_0;
		LCD.BytesPerPixel  = BytesPerPixel_0;
	}
	else if(layer == 1)	{
		LCD.LayerMemoryAdd = LCD_MemoryAdd + LCD_MemoryAdd_OFFSET;
		LCD.ColorMode      = ColorMode_1;
		LCD.BytesPerPixel  = BytesPerPixel_1;
	}
	LCD.Layer = layer;
#else
	LCD.LayerMemoryAdd = LCD_MemoryAdd;
	LCD.ColorMode      = ColorMode_0;
	LCD.BytesPerPixel  = BytesPerPixel_0;
	LCD.Layer = 0;
#endif
}


void LCD_SetColor(uint32_t Color) {
	uint16_t Alpha_Value = 0, Red_Value = 0, Green_Value = 0, Blue_Value = 0;
	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB565) {
		Red_Value   = (uint16_t)((Color&0x00F80000)>>8);
		Green_Value = (uint16_t)((Color&0x0000FC00)>>5);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);
		LCD.Color = (uint16_t)(Red_Value | Green_Value | Blue_Value);
	} else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB1555 ) {
		if( (Color & 0xFF000000) == 0 )	Alpha_Value = 0x0000;
		else Alpha_Value = 0x8000;
		Red_Value   = (uint16_t)((Color&0x00F80000)>>9);
		Green_Value = (uint16_t)((Color&0x0000F800)>>6);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>3);
		LCD.Color = (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB4444 ) {
		Alpha_Value = (uint16_t)((Color&0xf0000000)>>16);
		Red_Value   = (uint16_t)((Color&0x00F00000)>>12);
		Green_Value = (uint16_t)((Color&0x0000F000)>>8);
		Blue_Value  = (uint16_t)((Color&0x000000F8)>>4);
		LCD.Color = (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
	} else LCD.Color = Color;
}

void LCD_SetBackColor(uint32_t Color) {
	uint16_t Alpha_Value = 0, Red_Value = 0, Green_Value = 0, Blue_Value = 0;
	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB565	) {
		Red_Value   	= (uint16_t)((Color&0x00F80000)>>8);
		Green_Value 	= (uint16_t)((Color&0x0000FC00)>>5);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>3);
		LCD.BackColor	= (uint16_t)(Red_Value | Green_Value | Blue_Value);
	} else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB1555 ) {
		if( (Color & 0xFF000000) == 0 )	Alpha_Value = 0x0000;
		else Alpha_Value = 0x8000;
		Red_Value   	= (uint16_t)((Color&0x00F80000)>>9);
		Green_Value 	= (uint16_t)((Color&0x0000F800)>>6);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>3);
		LCD.BackColor 	= (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
	}
	else if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB4444 ) {
		Alpha_Value 	= (uint16_t)((Color&0xf0000000)>>16);
		Red_Value   	= (uint16_t)((Color&0x00F00000)>>12);
		Green_Value 	= (uint16_t)((Color&0x0000F000)>>8);
		Blue_Value  	= (uint16_t)((Color&0x000000F8)>>4);
		LCD.BackColor 	= (uint16_t)(Alpha_Value | Red_Value | Green_Value | Blue_Value);
	} else LCD.BackColor = Color;
}

void LCD_SetFont(pFONT *fonts) {
	LCD_Fonts = fonts;
}

void LCD_DisplayDirection(uint8_t direction) {
	LCD.Direction = direction;
}


void LCD_Clear(void) {
	DMA2D->CR	  &=	~(DMA2D_CR_START);
	DMA2D->CR		=	DMA2D_R2M;
	DMA2D->OPFCCR	=	LCD.ColorMode;
	DMA2D->OOR		=	0;
	DMA2D->OMAR		=	LCD.LayerMemoryAdd ;
	DMA2D->NLR		=	(LCD_Width<<16)|(LCD_Height);
	DMA2D->OCOLR	=	LCD.BackColor;
	while( LTDC->CDSR != 0X00000001) {}
	DMA2D->CR	  |=	DMA2D_CR_START;
	while (DMA2D->CR & DMA2D_CR_START) ;
}


void LCD_ClearRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	DMA2D->CR	  &=	~(DMA2D_CR_START);
	DMA2D->CR		=	DMA2D_R2M;
	DMA2D->OPFCCR	=	LCD.ColorMode;
	DMA2D->OCOLR	=	LCD.BackColor ;

	if(LCD.Direction == Direction_H) {
		DMA2D->OOR		=	LCD_Width - width;
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*(LCD_Width * y + x);
		DMA2D->NLR		=	(width<<16)|(height);
	} else {
		DMA2D->OOR		=	LCD_Width - height;
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*((LCD_Height - x - 1 - width)*LCD_Width + y);
		DMA2D->NLR		=	(width)|(height<<16);
	}
	DMA2D->CR	  |=	DMA2D_CR_START;
	while (DMA2D->CR & DMA2D_CR_START) ;
}


void LCD_DrawPoint(uint16_t x,uint16_t y,uint32_t color) {

	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB8888 )
		if (LCD.Direction == Direction_H)
			*(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*(x + y*LCD_Width) ) = color ;
		else if(LCD.Direction == Direction_V)
			*(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;

	else if ( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB888 ) {
		if (LCD.Direction == Direction_H) {
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) ) = color ;
			*(__IO uint8_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) + 2 ) = color>>16 ;
		} else if(LCD.Direction == Direction_V) {
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
			*(__IO uint8_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) +2) = color>>16 ;
		}
	} else {
		if (LCD.Direction == Direction_H)
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*(x + y*LCD_Width) ) = color ;
		else if(LCD.Direction == Direction_V)
			*(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*((LCD_Height - x - 1)*LCD_Width + y) ) = color ;
	}
}


uint32_t LCD_ReadPoint(uint16_t x,uint16_t y) {
	uint32_t color = 0;
	if( LCD.ColorMode == LTDC_PIXEL_FORMAT_ARGB8888 )
		if (LCD.Direction == Direction_H)
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*(x + y*LCD_Width) );
		else if(LCD.Direction == Direction_V)
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 4*((LCD_Height - x - 1)*LCD_Width + y) );

	else if ( LCD.ColorMode == LTDC_PIXEL_FORMAT_RGB888 )
		if (LCD.Direction == Direction_H)
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 3*(x + y*LCD_Width) ) &0x00ffffff;
		else if(LCD.Direction == Direction_V)
			color = *(__IO uint32_t*)( LCD.LayerMemoryAdd + 3*((LCD_Height - x - 1)*LCD_Width + y) ) &0x00ffffff;
	else
		if (LCD.Direction == Direction_H)
			color = *(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*(x + y*LCD_Width) );
		else if(LCD.Direction == Direction_V)
			color = *(__IO uint16_t*)( LCD.LayerMemoryAdd + 2*((LCD_Height - x - 1)*LCD_Width + y) );
	return color;
}


void LCD_DisplayChar(uint16_t x, uint16_t y,uint8_t c) {
	uint16_t  Xaddress = x;
	c = c - 32;
	for(uint16_t index = 0; index < LCD_Fonts->Sizes; index++) {
		uint8_t disChar = LCD_Fonts->pTable[c*LCD_Fonts->Sizes + index];
		for(uint16_t counter = 0; counter < 8; counter++) {
			if(disChar & 0x01) LCD_DrawPoint(Xaddress,y,LCD.Color);
			else LCD_DrawPoint(Xaddress, y, LCD.BackColor);
			disChar >>= 1;
			Xaddress++;
			if( (Xaddress - x)==LCD_Fonts->Width ) {
				Xaddress = x;
				y++;
				break;
			}
		}
	}
}

void LCD_DisplayString( uint16_t x, uint16_t y, char *p) {
	while ((x < LCD_Width) && (*p != 0)) {
		 LCD_DisplayChar( x,y,*p);
		 x += LCD_Fonts->Width;
		 p++;
	}
}


void LCD_SetTextFont(pChineseFont *fonts) {
	LCD_CHFonts = fonts;
	switch(fonts->Width ) {
		case 12:	LCD_Fonts = &Font12;	break;
		case 16:	LCD_Fonts = &Font16;	break;
		case 20:	LCD_Fonts = &Font20;	break;
		case 24:	LCD_Fonts = &Font24;	break;
		case 32:	LCD_Fonts = &Font32;	break;
		default: break;
	}
}

FIL		FontFile;
UINT 		FontFile_Num;
FRESULT 	FontFile_Res;

void GetGB2312Code ( uint8_t * pBuffer, uint16_t code) {
	uint8_t 	GB2312_H = 0,GB2312_L = 0;
	FSIZE_t fontOffset;
	GB2312_H = code >> 8;
	GB2312_L = code & 0x00FF;
	fontOffset = ((GB2312_H - 0xa1)*94 + GB2312_L - 0xa1) * LCD_CHFonts->Sizes	;				// ���㵱ǰ���ֵ�ƫ�Ƶ�ַ
	FontFile_Res = f_open(&FontFile,LCD_CHFonts->FontPath, FA_OPEN_EXISTING | FA_READ);		// ���ֿ��ļ�

	if ( FontFile_Res == FR_OK ) {
		f_lseek (&FontFile, fontOffset);
		FontFile_Res = f_read( &FontFile, pBuffer,LCD_CHFonts->Sizes, &FontFile_Num );
		f_close(&FontFile);
	} else{
		printf("Error: can not load GB2312");
	}
}

void LCD_DisplayChinese (uint16_t x,uint16_t y,uint16_t code) {
	uint8_t  charBuffer [512];
	uint16_t Xaddress = x;

	GetGB2312Code( charBuffer, code );		// ��ȡGB2312��ģ����

	for (uint16_t index = 0; index < LCD_CHFonts->Sizes; index++) {
		uint32_t charData = charBuffer[index];
		for (uint16_t counter = 0; counter < 8; counter ++ ) {
			if ( charData & 0x01) LCD_DrawPoint(Xaddress,y ,LCD.Color);
			else LCD_DrawPoint(Xaddress,y,LCD.BackColor);
			charData >>= 1;
			Xaddress++;
			if( (Xaddress - x) == LCD_CHFonts->Width) {
				Xaddress = x;
				y++;
				break;
			}
		}
	}
}


void LCD_DisplayText(uint16_t x, uint16_t y, char *pText){
	uint16_t code = 0;
	while (x + LCD_Fonts->Width <= LCD_Width && *pText != 0) {
		if (*pText <= 0x7F) {
			LCD_DisplayChar(x,y,*pText);
			x += LCD_Fonts->Width;
			pText++;
		} else {
			code = ( (*pText)<<8 ) + *(pText + 1);
			LCD_DisplayChinese(x,y,code);
			x += LCD_CHFonts->Width;
			pText += 2;
		}
	}
}



void LCD_ShowNumMode(uint8_t mode) {
	LCD.ShowNum_Mode = mode;
}

void  LCD_DisplayNumber( uint16_t x, uint16_t y, int32_t number, uint8_t len) {
	char   Number_Buffer[15];
	if( LCD.ShowNum_Mode == Fill_Zero)	sprintf( Number_Buffer , "%0.*d",len, number );
	else sprintf( Number_Buffer , "%*d",len, number );
	LCD_DisplayString( x, y,(char *)Number_Buffer);
}

void  LCD_DisplayDecimals( uint16_t x, uint16_t y, double decimals, uint8_t len, uint8_t decs) {
	char  Number_Buffer[20];
	if( LCD.ShowNum_Mode == Fill_Zero)	sprintf( Number_Buffer , "%0*.*lf",len,decs, decimals );
	else sprintf( Number_Buffer , "%*.*lf",len,decs, decimals );
	LCD_DisplayString( x, y,(char *)Number_Buffer);
}


void LCD_DrawImage(uint16_t x,uint16_t y,uint16_t width,uint16_t height,const uint8_t *pImage) {
	uint16_t  Xaddress = x; //ˮƽ����
	for(uint16_t i = 0; i <height; i++) {
		for(uint16_t j = 0; j <(float)width/8; j++) {
			uint8_t disChar = *pImage;
			for(uint16_t m = 0; m < 8; m++) {
				if(disChar & 0x01) LCD_DrawPoint(Xaddress,y,LCD.Color);
				else LCD_DrawPoint(Xaddress,y,LCD.BackColor);
				disChar >>= 1;
				Xaddress++;
				if( (Xaddress - x)==width ) {
					Xaddress = x;
					y++;
					break;
				}
			}
			pImage++;
		}
	}
}


#define ABS(X)  ((X) > 0 ? (X) : -(X))

void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0,
	yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0,
	curpixel = 0;
	deltax = ABS(x2 - x1);        /* The difference between the x's */
	deltay = ABS(y2 - y1);        /* The difference between the y's */
	x = x1;                       /* Start x off at the first pixel */
	y = y1;                       /* Start y off at the first pixel */

	if (x2 >= x1) {
		xinc1 = 1;
		xinc2 = 1;
	} else {
		xinc1 = -1;
		xinc2 = -1;
	}
	if (y2 >= y1) {
		yinc1 = 1;
		yinc2 = 1;
	} else {
		yinc1 = -1;
		yinc2 = -1;
	}

	if (deltax >= deltay) {
		xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
		yinc2 = 0;                  /* Don't change the y for every iteration */
		den = deltax;
		num = deltax / 2;
		numadd = deltay;
		numpixels = deltax;         /* There are more x-values than y-values */
	} else {
		xinc2 = 0;                  /* Don't change the x for every iteration */
		yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
		den = deltay;
		num = deltay / 2;
		numadd = deltax;
		numpixels = deltay;         /* There are more y-values than x-values */
	}
	for (curpixel = 0; curpixel <= numpixels; curpixel++) {
		LCD_DrawPoint(x,y,LCD.Color);             /* Draw the current pixel */
		num += numadd;              /* Increase the numerator by the top of the fraction */
		if (num >= den) {
			num -= den;               /* Calculate the new numerator value */
			x += xinc1;               /* Change the x as appropriate */
			y += yinc1;               /* Change the y as appropriate */
		}
		x += xinc2;                 /* Change the x as appropriate */
		y += yinc2;                 /* Change the y as appropriate */
	}
}

void LCD_DrawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	/* draw horizontal lines */
	LCD_DrawLine(x, y, x+width, y);
	LCD_DrawLine(x, y+height, x+width, y+height);

	/* draw vertical lines */
	LCD_DrawLine(x, y, x, y+height);
	LCD_DrawLine(x+width, y, x+width, y+height);
}

void LCD_DrawCircle(uint16_t x, uint16_t y, uint16_t r) {
	int Xadd = -r, Yadd = 0, err = 2-2*r, e2;
	do {
		LCD_DrawPoint(x-Xadd,y+Yadd,LCD.Color);
		LCD_DrawPoint(x+Xadd,y+Yadd,LCD.Color);
		LCD_DrawPoint(x+Xadd,y-Yadd,LCD.Color);
		LCD_DrawPoint(x-Xadd,y-Yadd,LCD.Color);

		e2 = err;
		if (e2 <= Yadd) {
			err += ++Yadd*2+1;
			if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
		}
		if (e2 > Xadd) err += ++Xadd*2+1;
    }
    while (Xadd <= 0);

}

void LCD_DrawEllipse(int x, int y, int r1, int r2) {
	int Xadd = -r1, Yadd = 0, err = 2-2*r1, e2;
	float K = 0, rad1 = 0, rad2 = 0;
	rad1 = r1;
	rad2 = r2;

	if (r1 > r2) {
		do {
			K = (float)(rad1/rad2);
			LCD_DrawPoint(x-Xadd,y+(uint16_t)(Yadd/K),LCD.Color);
			LCD_DrawPoint(x+Xadd,y+(uint16_t)(Yadd/K),LCD.Color);
			LCD_DrawPoint(x+Xadd,y-(uint16_t)(Yadd/K),LCD.Color);
			LCD_DrawPoint(x-Xadd,y-(uint16_t)(Yadd/K),LCD.Color);
			e2 = err;
			if (e2 <= Yadd) {
				err += ++Yadd*2+1;
				if (-Xadd == Yadd && e2 <= Xadd) e2 = 0;
			}
			if (e2 > Xadd) err += ++Xadd*2+1;
		} while (Xadd <= 0);
	} else {
		Yadd = -r2;
		Xadd = 0;
		do {
			K = (float)(rad2/rad1);
			LCD_DrawPoint(x-(uint16_t)(Xadd/K),y+Yadd,LCD.Color);
			LCD_DrawPoint(x+(uint16_t)(Xadd/K),y+Yadd,LCD.Color);
			LCD_DrawPoint(x+(uint16_t)(Xadd/K),y-Yadd,LCD.Color);
			LCD_DrawPoint(x-(uint16_t)(Xadd/K),y-Yadd,LCD.Color);

			e2 = err;
			if (e2 <= Xadd) {
				err += ++Xadd*3+1;
				if (-Yadd == Xadd && e2 <= Yadd) e2 = 0;
			}
			if (e2 > Yadd) err += ++Yadd*3+1;
		} while (Yadd <= 0);
	}
}

void LCD_FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	DMA2D->CR	  &=	~(DMA2D_CR_START);
	DMA2D->CR		=	DMA2D_R2M;
	DMA2D->OPFCCR	=	LCD.ColorMode;
	DMA2D->OCOLR	=	LCD.Color;
	if(LCD.Direction == Direction_H) {
		DMA2D->OOR		=	LCD_Width - width;
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*(LCD_Width * y + x);
		DMA2D->NLR		=	(width<<16)|(height);
	} else {
		DMA2D->OOR		=	LCD_Width - height;
		DMA2D->OMAR		=	LCD.LayerMemoryAdd + LCD.BytesPerPixel*((LCD_Height - x - 1 - width)*LCD_Width + y);
		DMA2D->NLR		=	(width)|(height<<16);
	}
	DMA2D->CR	  |=	DMA2D_CR_START;
	while (DMA2D->CR & DMA2D_CR_START) ;
}

void LCD_FillCircle(uint16_t x, uint16_t y, uint16_t r) {
	int32_t D = 3 - (r << 1);
	uint32_t CurX = 0;
	uint32_t CurY = r;
	while (CurX <= CurY) {
		if(CurY > 0) {
			LCD_DrawLine(x - CurX, y - CurY,x - CurX,y - CurY + 2*CurY);
			LCD_DrawLine(x + CurX, y - CurY,x + CurX,y - CurY + 2*CurY);
		}
		if(CurX > 0) {
			LCD_DrawLine(x - CurY, y - CurX,x - CurY,y - CurX + 2*CurX);
			LCD_DrawLine(x + CurY, y - CurX,x + CurY,y - CurX + 2*CurX);
		}
		if (D < 0) D += (CurX << 2) + 6;
		else {
			D += ((CurX - CurY) << 2) + 10;
			CurY--;
		}
		CurX++;
	}
	LCD_DrawCircle(x, y, r);
}
/* USER CODE END 1 */
