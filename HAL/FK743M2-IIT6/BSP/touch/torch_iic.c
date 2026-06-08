#include "torch_iic.h"
#include "gpio.h"

volatile TouchStructure touchInfo;

void Touch_IIC_Delay(uint32_t a){
    volatile uint16_t i;
    while (a --)
        for (i = 0; i < 8; i++);
}

void Touch_INT_Out(void){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin   = Touch_INT_PIN ;

    HAL_GPIO_Init(Touch_INT_PORT, &GPIO_InitStruct);
}

void Touch_INT_In(void){
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin   = Touch_INT_PIN ;

    HAL_GPIO_Init(Touch_INT_PORT, &GPIO_InitStruct);

}


void Touch_IIC_Start(void){
    Touch_IIC_SDA(1);
    Touch_IIC_SCL(1);
    Touch_IIC_Delay(IIC_DelayVaule);

    Touch_IIC_SDA(0);
    Touch_IIC_Delay(IIC_DelayVaule);
    Touch_IIC_SCL(0);
    Touch_IIC_Delay(IIC_DelayVaule);
}

void Touch_IIC_Stop(void){
    Touch_IIC_SCL(0);
    Touch_IIC_Delay(IIC_DelayVaule);
    Touch_IIC_SDA(0);
    Touch_IIC_Delay(IIC_DelayVaule);

    Touch_IIC_SCL(1);
    Touch_IIC_Delay(IIC_DelayVaule);
    Touch_IIC_SDA(1);
    Touch_IIC_Delay(IIC_DelayVaule);
}

void Touch_IIC_ACK(void){
    Touch_IIC_SCL(0);
    Touch_IIC_Delay(IIC_DelayVaule);
    Touch_IIC_SDA(0);
    Touch_IIC_Delay(IIC_DelayVaule);
    Touch_IIC_SCL(1);
    Touch_IIC_Delay(IIC_DelayVaule);

    Touch_IIC_SCL(0);
    Touch_IIC_SDA(1);

    Touch_IIC_Delay(IIC_DelayVaule);

}


void Touch_IIC_NoACK(void){
    Touch_IIC_SCL(0);
    Touch_IIC_Delay(IIC_DelayVaule);
    Touch_IIC_SDA(1);
    Touch_IIC_Delay(IIC_DelayVaule);
    Touch_IIC_SCL(1);
    Touch_IIC_Delay(IIC_DelayVaule);

    Touch_IIC_SCL(0);
    Touch_IIC_Delay(IIC_DelayVaule);
}


uint8_t Touch_IIC_WaitACK(void){
    Touch_IIC_SDA(1);
    Touch_IIC_Delay(IIC_DelayVaule);
    Touch_IIC_SCL(1);
    Touch_IIC_Delay(IIC_DelayVaule);

    if( HAL_GPIO_ReadPin(Touch_IIC_SDA_PORT,Touch_IIC_SDA_PIN) != 0) {
        Touch_IIC_SCL(0);
        Touch_IIC_Delay( IIC_DelayVaule );
        return ACK_ERR;	//?????
    }else {
        Touch_IIC_SCL(0);
        Touch_IIC_Delay( IIC_DelayVaule );
        return ACK_OK;
    }
}


uint8_t Touch_IIC_WriteByte(uint8_t IIC_Data){
    uint8_t i;

    for (i = 0; i < 8; i++){
        Touch_IIC_SDA(IIC_Data & 0x80);

        Touch_IIC_Delay( IIC_DelayVaule );
        Touch_IIC_SCL(1);
        Touch_IIC_Delay( IIC_DelayVaule );
        Touch_IIC_SCL(0);
        if(i == 7)
            Touch_IIC_SDA(1);
        IIC_Data <<= 1;
    }
    return Touch_IIC_WaitACK(); //????υτ???
}


uint8_t Touch_IIC_ReadByte(uint8_t ACK_Mode){
    uint8_t IIC_Data = 0;
    uint8_t i = 0;

    for (i = 0; i < 8; i++){
        IIC_Data <<= 1;

        Touch_IIC_SCL(1);
        Touch_IIC_Delay( IIC_DelayVaule );
        IIC_Data |= (HAL_GPIO_ReadPin(Touch_IIC_SDA_PORT,Touch_IIC_SDA_PIN) & 0x01);
        Touch_IIC_SCL(0);
        Touch_IIC_Delay( IIC_DelayVaule );
    }

    if ( ACK_Mode == 1 )
        Touch_IIC_ACK();
    else
        Touch_IIC_NoACK();

    return IIC_Data;
}

void GT9XX_Reset(void){
    Touch_INT_Out();

    HAL_GPIO_WritePin(Touch_INT_PORT,Touch_INT_PIN,GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Touch_RST_PORT,Touch_RST_PIN,GPIO_PIN_SET);
    Touch_IIC_Delay(10000);

    HAL_GPIO_WritePin(Touch_RST_PORT,Touch_RST_PIN,GPIO_PIN_RESET);
    Touch_IIC_Delay(150000);
    HAL_GPIO_WritePin(Touch_RST_PORT,Touch_RST_PIN,GPIO_PIN_SET);
    Touch_IIC_Delay(350000);
    Touch_INT_In();
    Touch_IIC_Delay(20000);
}

uint8_t GT9XX_WriteHandle (uint16_t addr){
    uint8_t status;

    Touch_IIC_Start();
    if( Touch_IIC_WriteByte(GT9XX_IIC_WADDR) == ACK_OK )
        if( Touch_IIC_WriteByte((uint8_t)(addr >> 8)) == ACK_OK )
            if( Touch_IIC_WriteByte((uint8_t)(addr)) != ACK_OK )
                status = ERROR;

    status = SUCCESS;
    return status;
}

uint8_t GT9XX_WriteData (uint16_t addr,uint8_t value){
    uint8_t status;

    Touch_IIC_Start();

    if( GT9XX_WriteHandle(addr) == SUCCESS)
        if (Touch_IIC_WriteByte(value) != ACK_OK) status = ERROR;


    Touch_IIC_Stop();
    status = SUCCESS;
    return status;
}

uint8_t GT9XX_WriteReg (uint16_t addr, uint8_t cnt, uint8_t *value){
    uint8_t status;
    uint8_t i;

    Touch_IIC_Start();

    if( GT9XX_WriteHandle(addr) == SUCCESS){
        for(i = 0 ; i < cnt; i++)
            Touch_IIC_WriteByte(value[i]);
        Touch_IIC_Stop();
        status = SUCCESS;
    }else{
        Touch_IIC_Stop();
        status = ERROR;
    }
    return status;
}

uint8_t GT9XX_ReadReg (uint16_t addr, uint8_t cnt, uint8_t *value){
    uint8_t status;
    uint8_t i;

    status = ERROR;
    Touch_IIC_Start();

    if( GT9XX_WriteHandle(addr) == SUCCESS) {
        Touch_IIC_Start();
        if (Touch_IIC_WriteByte(GT9XX_IIC_RADDR) == ACK_OK){
            for(i = 0 ; i < cnt; i++){
                if (i == (cnt - 1))
                    value[i] = Touch_IIC_ReadByte(0);
                else
                    value[i] = Touch_IIC_ReadByte(1);
            }
            Touch_IIC_Stop();
            status = SUCCESS;
        }
    }
    Touch_IIC_Stop();
    return (status);
}

uint8_t Touch_Init(void){
    uint8_t GT9XX_Info[11];
    uint8_t cfgVersion = 0;
    GT9XX_Reset();

    HAL_GPIO_WritePin(Touch_IIC_SCL_PORT, Touch_IIC_SCL_PIN, GPIO_PIN_SET);		// SCL???????
    HAL_GPIO_WritePin(Touch_IIC_SDA_PORT, Touch_IIC_SDA_PIN, GPIO_PIN_SET);    // SDA???????
    HAL_GPIO_WritePin(Touch_INT_PORT, 	  Touch_INT_PIN,     GPIO_PIN_RESET);  // INT???????
    HAL_GPIO_WritePin(Touch_RST_PORT,     Touch_RST_PIN,     GPIO_PIN_SET);    // RST???????


    GT9XX_ReadReg (GT9XX_ID_ADDR,11,GT9XX_Info);
    GT9XX_ReadReg (GT9XX_CFG_ADDR,1,&cfgVersion);

    if( GT9XX_Info[0] == '9' )
        return SUCCESS;
    else
        return ERROR;

}

void Touch_Scan(void){
    uint8_t  touchData[2 + 8 * TOUCH_MAX ];
    uint8_t  i = 0;

    GT9XX_ReadReg (GT9XX_READ_ADDR,2 + 8 * TOUCH_MAX ,touchData);
    GT9XX_WriteData (GT9XX_READ_ADDR,0);
    touchInfo.num = touchData[0] & 0x0f;

    if ( (touchInfo.num >= 1) && (touchInfo.num <=5) ) {
        for(i=0;i<touchInfo.num;i++){
            touchInfo.y[i] = (touchData[5+8*i]<<8) | touchData[4+8*i];	// ???Y????
            touchInfo.x[i] = (touchData[3+8*i]<<8) | touchData[2+8*i];	//	???X????
        }
        touchInfo.flag = 1;
    }
    else touchInfo.flag = 0;
}
