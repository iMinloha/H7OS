#include "xTaskInit.h"
#include "fatfs.h"
#include "u_stdio.h"
#include "sdmmc.h"
#include "quadspi.h"
#include "RAMFS.h"
#include "adc.h"

// 初始化全局任务
void taskGlobalInit(){
    // 输出SD卡信息， 确保SD卡正常工作
    HAL_SD_CardInfoTypeDef  SDCardInfo;
    HAL_SD_CardCIDTypeDef SDCard_CID;

    HAL_SD_GetCardCID(&hsd1,&SDCard_CID);
    HAL_SD_GetCardInfo(&hsd1,&SDCardInfo);
    uint64_t CardCap=(uint64_t)(SDCardInfo.LogBlockNbr)*(uint64_t)(SDCardInfo.LogBlockSize);	//计算SD卡容量
    u_print("SD card Drive Capacitor: %D MB\r\n", (uint32_t)(CardCap>>20));

    // CPU采样初始化
    HAL_ADCEx_Calibration_Start(&hadc3,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED);
    HAL_ADC_Start(&hadc3);  /* 启动ADC3的转换 */

    // QSPI Flash初始化，擦除所有数据
    if(QSPI_W25Qxx_BlockErase_32K(0) != QSPI_W25Qxx_OK)
        u_print("Erase Failed\n");
    else u_print("QSPI Flash Succeed, ID: %d\n", QSPI_W25Qxx_ReadID());

    // RAMFS初始化
    DrTInit();
    // CPU结构体初始化(用于标注CPU的信息)
    createCPU();
}

/*** 函数书写声明 **
 *  @note: 该函数用于初始化SD卡，如果SD卡未挂载，则尝试格式化SD卡
 *      本项目使用CubeMX生成的Fatfs，因使用了Freertos，Fatfs必须使用FreeRTOS的消息队列
 *      所以需要在osKernelStart()之前初始化Fatfs
 *      注意：需要开机MDMA才可以正常使用FATFS的f_mkfs函数
 * */
void QueueInit(void const * argument){
    // SD卡挂载FATFS
    FRESULT FSRes = f_mount(&SDFatFS,SDPath,1);
    BYTE work[_MAX_SS];
    // 如果挂载失败，尝试格式化SD卡
    if (FSRes != FR_OK){
        // 创建FAT32文件系统
        FSRes = f_mkfs(SDPath, FM_FAT32, 0, work, sizeof work);
        // 判断是否初始化成果
        if (FSRes == FR_OK) {
            // 初始化成功，重新挂载
            u_print("SD card init succeed\r\n");
            f_mount(&SDFatFS,SDPath,1);
        }
        // 初始化失败，提示用户更换SD卡
        else u_print("Init Faild, please replace SD card\r\n");
    }
    // 初始化成功，提示用户
    else u_print("SD card Succeed\r\n");
    // 一次性初始化完成，挂起初始化任务
    vTaskSuspend(xTaskInitHandle);
}