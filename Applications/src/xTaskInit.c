#include "xTaskInit.h"
#include "fatfs.h"
#include "stdio.h"
#include "sdmmc.h"
#include "quadspi.h"
#include "RAMFS.h"
#include "adc.h"
#include "TaskHead.h"
#include "usbd_cdc_if.h"
#include "usart.h"
#include "Periph/FS_Serial.h"
#include "memctl.h"

uint8_t Sd_status = 0;
extern FATFS SDFatFS; /* File system object for SD logical drive */


// 初始化全局任务
void taskGlobalInit(){
    // RAMFS初始化
    DrTInit();
    HAL_SD_CardInfoTypeDef SDCardInfo;
    HAL_SD_CardCIDTypeDef SDCard_CID;

    HAL_SD_GetCardCID(&hsd1,&SDCard_CID);
    HAL_SD_GetCardInfo(&hsd1,&SDCardInfo);
    uint64_t CardCap = (uint64_t)(SDCardInfo.LogBlockNbr) * (uint64_t)(SDCardInfo.LogBlockSize);	//计算SD卡容量
    /* FATFS_LinkDriver 已在 MX_FATFS_Init() 中调用, 此处不重复 */

    // 判断SD卡是否初始化
    if (CardCap > 0) {
        printf("[xTaskInit]: SD Card Capacity: %lldMB\n", CardCap / 1024 / 1024);
        Sd_status = 1;
    } else {
        printf("[xTaskInit]: SD Card Not Found\n");
        Sd_status = 0;
    }

    // CPU采样初始化
    HAL_ADCEx_Calibration_Start(&hadc3,ADC_CALIB_OFFSET,ADC_SINGLE_ENDED);
    HAL_ADC_Start(&hadc3);  /* 启动ADC3的转换 */

    // QSPI Flash初始化，擦除所有数据
    if(QSPI_W25Qxx_Init() != QSPI_W25Qxx_OK) printf("Check W25Qxx Failed\n");
    else {
        addDevice("mnt", &hqspi, "QSPI", "Quad SPI", DEVICE_STORAGE, DEVICE_ON, NULL);
        printf("[xTaskInit]: QSPI Flash Succeed, ID: %lx\n", QSPI_W25Qxx_ReadID());
    }
    /* CPU 设备由 devices_init() -> cpu_device_init() 统一注册,
       不再在此单独调用 createCPU() */
}

/*** 函数书写声明 **
 *  @note: 该函数用于初始化SD卡，如果SD卡未挂载，则尝试格式化SD卡
 *      本项目使用CubeMX生成的Fatfs，因使用了Freertos，Fatfs必须使用FreeRTOS的消息队列
 *      所以需要在osKernelStart()之前初始化Fatfs
 *      注意：需要开机MDMA才可以正常使用FATFS的f_mkfs函数
 * */
void QueueInit(void const * argument){
    FS_Deserialize();
    if (Sd_status == 1) {
        FRESULT FSRes = f_mount(&SDFatFS, SDPath, 1);
        // 如果挂载失败，尝试格式化SD卡
        if (FSRes != FR_OK) {
            // 创建FAT32文件系统
            BYTE work[_MAX_SS];
            FSRes = f_mkfs(SDPath, FM_FAT32, 0, work, sizeof(work));
            // 判断是否初始化成果
            if (FSRes == FR_OK) {
                // 初始化成功，重新挂载
                printf("[xTaskInit]: Fatfs Succeed\r\n");
                addDevice("mnt", NULL, "SDcard", "FAT file system", FILE_SYSTEM, DEVICE_ON, NULL);
                f_mount(&SDFatFS, SDPath, 1);
            }
            // 初始化失败，提示用户更换SD卡
            else printf("\n[xTaskInit]: SD Init Faild, please replace SD card\r\n");

        } else {
            addDevice("mnt", &SDFatFS, "SDcard", "FAT file system", FILE_SYSTEM, DEVICE_ON, NULL);
            printf("[xTaskInit]: Fatfs Succeed\r\n");
            /* 修复: FatFs 未正确计算 database */
            if (SDFatFS.fs_type == FS_FAT32) {
                DWORD db = SDFatFS.fatbase + SDFatFS.n_fats * SDFatFS.fsize;
                if (SDFatFS.database != db) {
                    printf("[xTaskInit]: fix database %lu -> %lu\n", SDFatFS.database, db);
                    SDFatFS.database = db;
                }
            }
        }
    }
    // 一次性初始化完成，挂起初始化任务
    while (1) osDelay(1000);
}