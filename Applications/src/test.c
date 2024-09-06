#include "test.h"
#include "cmsis_os.h"
#include "u_stdio.h"
#include "TaskHead.h"

// 输出SD卡信息
#if 0
void printf_sdcard_info(void){
    HAL_SD_CardInfoTypeDef  SDCardInfo;
    uint64_t CardCap;      	//SD卡容量
    HAL_SD_CardCIDTypeDef SDCard_CID;

    HAL_SD_GetCardCID(&hsd1,&SDCard_CID);	//获取CID
    HAL_SD_GetCardInfo(&hsd1,&SDCardInfo);                    //获取SD卡信息
    CardCap=(uint64_t)(SDCardInfo.LogBlockNbr)*(uint64_t)(SDCardInfo.LogBlockSize);	//计算SD卡容量
    switch(SDCardInfo.CardType)
    {
        case CARD_SDSC:
        {
            if(SDCardInfo.CardVersion == CARD_V1_X)
                u_print("Card Type:SDSC V1\r\n");
            else if(SDCardInfo.CardVersion == CARD_V2_X)
                u_print("Card Type:SDSC V2\r\n");
        }
            break;
        case CARD_SDHC_SDXC:u_print("Card Type:SDHC\r\n");break;
        default:break;
    }
    // SD的各种信息输出，用于确定SD卡是否正常工作
    u_print("Card ManufacturerID: %d \r\n",SDCard_CID.ManufacturerID);				//制造商ID
    u_print("CardVersion:         %d \r\n",(uint32_t)(SDCardInfo.CardVersion));		//卡版本号
    u_print("Class:               %d \r\n",(uint32_t)(SDCardInfo.Class));		    //
    u_print("Card RCA(RelCardAdd):%d \r\n",SDCardInfo.RelCardAdd);					//卡相对地址
    u_print("Card BlockNbr:       %d \r\n",SDCardInfo.BlockNbr);						//块数量
    u_print("Card BlockSize:      %d \r\n",SDCardInfo.BlockSize);					//块大小
    u_print("LogBlockNbr:         %d \r\n",(uint32_t)(SDCardInfo.LogBlockNbr));		//逻辑块数量
    u_print("LogBlockSize:        %d \r\n",(uint32_t)(SDCardInfo.LogBlockSize));		//逻辑块大小
    u_print("Card Capacity:       %d MB\r\n",(uint32_t)(CardCap>>20));				//卡容量
}
#endif

extern Task_t xTest;

void testFunc(){
    u_print("OK \r\n");
    while(1){
        TaskTickStart(xTest);
        u_print("Hi! \r\n");
        osDelay(1000);
        TaskTickEnd(xTest);
    }

}