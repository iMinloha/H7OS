#include <string.h>
#include "test.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "TaskHead.h"
#include "quadspi.h"
#include "memctl.h"
#include "init.d/DrT/DrT.h"
#include "init.d/CPU/CS.h"


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

typedef struct test{
    int a;
    float b;
} *test_p;

typedef void * Everything;

void testFunc(){

//    test_p test_ptr = kernel_alloc(sizeof(struct test));
//    test_ptr->a = 2;
//    test_ptr->b = 2.0f;
//    Everything everything = (Everything) test_ptr;
//    uint8_t tmp[2] = {11, 22};
//    uint8_t tmp2[2] = {0};
//    CS_push("HI");
//    CS_save();
//    QSPI_W25Qxx_WriteBuffer(tmp, 0, 2);


    while(1){
        TaskTickStart(xTest);
//        QSPI_W25Qxx_ReadBuffer(tmp2, 0, 2);
//        printf("tmp2[0]: %d, tmp2[1]: %d\n", tmp2[0], tmp2[1]);
//        CS_load();
//        execCMD("mkdir test/test1");
//        execCMD("mkdir test/test2");
//        execCMD("rm -r test");
        osDelay(1000);
//        CS_list();
        TaskTickEnd(xTest);
    }
}