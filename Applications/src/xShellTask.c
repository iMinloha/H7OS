#include "xShellTask.h"
#include "u_stdio.h"
#include "cmsis_os.h"
#include "quadspi.h"
#include "RAMFS.h"
#include "fatfs.h"
#include "sdmmc.h"

void taskGlobalInit(){
    HAL_SD_CardInfoTypeDef  SDCardInfo;
    HAL_SD_CardCIDTypeDef SDCard_CID;

    HAL_SD_GetCardCID(&hsd1,&SDCard_CID);
    HAL_SD_GetCardInfo(&hsd1,&SDCardInfo);
    uint64_t CardCap=(uint64_t)(SDCardInfo.LogBlockNbr)*(uint64_t)(SDCardInfo.LogBlockSize);	//¼ÆËãSD¿¨ÈÝÁ¿
    u_print("SD card Drive Capacitor: %D MB\r\n", (uint32_t)(CardCap>>20));

    if(QSPI_W25Qxx_BlockErase_32K(0) != QSPI_W25Qxx_OK)
        u_print("Erase Failed\n");
    else u_print("QSPI Flash Succeed, ID: %d\n", QSPI_W25Qxx_ReadID());

    DrTInit();
}


void taskShellInit(){

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
}


void taskLoop(){
    osDelay(1000);
}