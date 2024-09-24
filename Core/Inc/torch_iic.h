#ifndef H7OS_TORCH_IIC_H
#define H7OS_TORCH_IIC_H

#include <stdint-gcc.h>

#define TOUCH_MAX   5

typedef struct{
    uint8_t  flag;
    uint8_t  num;
    uint16_t x[TOUCH_MAX];
    uint16_t y[TOUCH_MAX];
}TouchStructure;

extern volatile TouchStructure touchInfo;

#define Touch_IIC_SCL_PORT   			 GPIOI
#define Touch_IIC_SCL_PIN     		 GPIO_PIN_11

#define Touch_IIC_SDA_PORT   			 GPIOI
#define Touch_IIC_SDA_PIN    			 GPIO_PIN_8

#define Touch_INT_PORT   				 GPIOG
#define Touch_INT_PIN    				 GPIO_PIN_3

#define Touch_RST_PORT   				 GPIOH
#define Touch_RST_PIN    				 GPIO_PIN_4


#define ACK_OK  	1
#define ACK_ERR 	0

#define IIC_DelayVaule  20

#define Touch_IIC_SCL(a)	if (a)	\
										HAL_GPIO_WritePin(Touch_IIC_SCL_PORT, Touch_IIC_SCL_PIN, GPIO_PIN_SET); \
									else		\
										HAL_GPIO_WritePin(Touch_IIC_SCL_PORT, Touch_IIC_SCL_PIN, GPIO_PIN_RESET)

#define Touch_IIC_SDA(a)	if (a)	\
										HAL_GPIO_WritePin(Touch_IIC_SDA_PORT, Touch_IIC_SDA_PIN, GPIO_PIN_SET); \
									else		\
										HAL_GPIO_WritePin(Touch_IIC_SDA_PORT, Touch_IIC_SDA_PIN, GPIO_PIN_RESET)

void 		Touch_IIC_Delay(uint32_t a);
void		Touch_INT_Out(void);
void		Touch_INT_In(void);
void 		Touch_IIC_Start(void);
void 		Touch_IIC_Stop(void);
void 		Touch_IIC_ACK(void);
void 		Touch_IIC_NoACK(void);
uint8_t 	Touch_IIC_WaitACK(void);
uint8_t	Touch_IIC_WriteByte(uint8_t IIC_Data);
uint8_t 	Touch_IIC_ReadByte(uint8_t ACK_Mode);

#define GT9XX_IIC_RADDR 0xBB
#define GT9XX_IIC_WADDR 0xBA
#define GT9XX_CFG_ADDR 	0x8047
#define GT9XX_READ_ADDR 0x814E
#define GT9XX_ID_ADDR 	0x8140

uint8_t 	Touch_Init(void);
void 		Touch_Scan(void);
void  	GT9XX_Reset(void);
void 		GT9XX_SendCfg(void);
void 		GT9XX_ReadCfg(void);


#endif //H7OS_TORCH_IIC_H
