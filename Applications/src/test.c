#include "test.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "TaskHead.h"


#if 0
void printf_sdcard_info(void){
    HAL_SD_CardInfoTypeDef  SDCardInfo;
    uint64_t CardCap;      	//SD������
    HAL_SD_CardCIDTypeDef SDCard_CID;

    HAL_SD_GetCardCID(&hsd1,&SDCard_CID);	//��ȡCID
    HAL_SD_GetCardInfo(&hsd1,&SDCardInfo);                    //��ȡSD����Ϣ
    CardCap=(uint64_t)(SDCardInfo.LogBlockNbr)*(uint64_t)(SDCardInfo.LogBlockSize);	//����SD������
    switch(SDCardInfo.CardType)
    {
        case CARD_SDSC:
        {
            if(SDCardInfo.CardVersion == CARD_V1_X)
                printf("Card Type:SDSC V1\r\n");
            else if(SDCardInfo.CardVersion == CARD_V2_X)
                printf("Card Type:SDSC V2\r\n");
        }
            break;
        case CARD_SDHC_SDXC:printf("Card Type:SDHC\r\n");break;
        default:break;
    }
    // SD�ĸ�����Ϣ���������ȷ��SD���Ƿ���������
    printf("Card ManufacturerID: %d \r\n",SDCard_CID.ManufacturerID);				//������ID
    printf("CardVersion:         %d \r\n",(uint32_t)(SDCardInfo.CardVersion));		//���汾��
    printf("Class:               %d \r\n",(uint32_t)(SDCardInfo.Class));		    //
    printf("Card RCA(RelCardAdd):%d \r\n",SDCardInfo.RelCardAdd);					//����Ե�ַ
    printf("Card BlockNbr:       %d \r\n",SDCardInfo.BlockNbr);						//������
    printf("Card BlockSize:      %d \r\n",SDCardInfo.BlockSize);					//���С
    printf("LogBlockNbr:         %d \r\n",(uint32_t)(SDCardInfo.LogBlockNbr));		//�߼�������
    printf("LogBlockSize:        %d \r\n",(uint32_t)(SDCardInfo.LogBlockSize));		//�߼����С
    printf("Card Capacity:       %d MB\r\n",(uint32_t)(CardCap>>20));				//������
}
#endif

extern Task_t xTest;

void testFunc(){
//    printf("OK \r\n");
    while(1){
        TaskTickStart(xTest);
//        printf("Hi! \r\n");
        osDelay(1000);
        TaskTickEnd(xTest);
    }

}