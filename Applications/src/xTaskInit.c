#include "xTaskInit.h"
#include "fatfs.h"
#include "u_stdio.h"
#include "sdmmc.h"
#include "quadspi.h"
#include "RAMFS.h"
#include "adc.h"

void taskGlobalInit(){

    HAL_SD_CardInfoTypeDef  SDCardInfo;
    HAL_SD_CardCIDTypeDef SDCard_CID;

    HAL_SD_GetCardCID(&hsd1,&SDCard_CID);
    HAL_SD_GetCardInfo(&hsd1,&SDCardInfo);
    uint64_t CardCap=(uint64_t)(SDCardInfo.LogBlockNbr)*(uint64_t)(SDCardInfo.LogBlockSize);	//计算SD卡容量
    u_print("SD card Drive Capacitor: %D MB\r\n", (uint32_t)(CardCap>>20));

    // CPU采样初始化
    HAL_ADCEx_Calibration_Start(&hadc3,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED);

    // QSPI Flash初始化
    if(QSPI_W25Qxx_BlockErase_32K(0) != QSPI_W25Qxx_OK)
        u_print("Erase Failed\n");
    else u_print("QSPI Flash Succeed, ID: %d\n", QSPI_W25Qxx_ReadID());

    DrTInit();

    createCPU();
}

void QueueInit(void const * argument){
    FRESULT FSRes = f_mount(&SDFatFS,SDPath,1);
    BYTE work[_MAX_SS];
    if (FSRes != FR_OK){
        FSRes = f_mkfs(SDPath, FM_FAT32, 0, work, sizeof work);
        if (FSRes == FR_OK) {
            u_print("SD card init succeed\r\n");
            f_mount(&SDFatFS,SDPath,1);
        }
        else u_print("Init Faild, please replace SD card\r\n");
    }
    else u_print("SD card Succeed\r\n");

    vTaskSuspend(xTaskInitHandle);
}