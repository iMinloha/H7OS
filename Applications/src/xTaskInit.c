/**
 * @file    xTaskInit.c
 * @brief   全局任务初始化 — SD卡检测, QSPI Flash 注册, ADC 启动
 *          Global task init — SD card detect, QSPI Flash register, ADC startup
 *
 * 所有硬件访问通过 Platform API, 不直接调用 HAL。
 * All hardware access through Platform API, no direct HAL calls.
 */

#include "xTaskInit.h"
#include "fatfs.h"
#include "stdio.h"
#include "RAMFS.h"
#include "TaskHead.h"
#include "usbd_cdc_if.h"
#include "Periph/FS_Serial.h"
#include "memctl.h"
#include "platform.h"

uint8_t Sd_status = 0;
extern FATFS SDFatFS; /* SD 逻辑驱动器文件系统对象 / File system object for SD logical drive */

/**
 * @brief  FreeRTOS 启动前全局初始化
 *         Global initialization before FreeRTOS starts
 *
 * 执行顺序 / Execution order:
 *   1. DrTInit() — 设备树 + RAMFS + devices_init() + shell 命令注册
 *   2. SD 卡检测 / SD card detection
 *   3. ADC3 校准启动 / ADC3 calibration & start
 *   4. QSPI Flash 初始化 / QSPI Flash init
 */
void taskGlobalInit(){
    /* ── DrT 设备树 + RAMFS + 外设注册 + Shell 命令 ───────── */
    /*    DrT device tree + RAMFS + peripheral registration + shell commands */
    DrTInit();

    /* ── SD 卡检测 / SD card detection ────────────────────── */
    Platform_SD_Info sd_info;
    if (Platform_SD_GetInfo(&sd_info) && sd_info.capacity_bytes > 0) {
        printf("[xTaskInit]: SD Card Capacity: %lldMB\n",
               (long long)(sd_info.capacity_bytes / 1024 / 1024));
        Sd_status = 1;
    } else {
        printf("[xTaskInit]: SD Card Not Found\n");
        Sd_status = 0;
    }

    /* ── QSPI Flash 初始化并注册到 DrT ────────────────────── */
    /*    QSPI Flash init and register to DrT */
    if (Platform_QSPI_Init() != 0) {
        printf("Check W25Qxx Failed\n");
    } else {
        addDevice("mnt", Platform_QSPI_GetHandle(),
                  "QSPI", "Quad SPI", DEVICE_STORAGE, DEVICE_ON, NULL);
        printf("[xTaskInit]: QSPI Flash Succeed, ID: %lx\n",
               (unsigned long)Platform_QSPI_ReadID());
    }
}

/**
 * @brief  FreeRTOS 启动后初始化 (队列任务)
 *         Post-FreeRTOS initialization (queue task)
 *
 * 负责 / Responsible for:
 *   - 从 QSPI 反序列化 RAMFS / Deserialize RAMFS from QSPI
 *   - 挂载 SD 卡 FatFs / Mount SD card FatFs
 *   - 如果挂载失败尝试格式化 / Try format if mount fails
 *
 * @note  FatFs 使用 FreeRTOS 消息队列, 必须在 osKernelStart() 之后操作
 *        FatFs uses FreeRTOS message queues, must operate after osKernelStart()
 * @note  格式化需要 MDMA 使能 / Format requires MDMA enabled
 */
void QueueInit(void const * argument){
    FS_Deserialize();

    if (Sd_status == 1) {
        const char *sd_path = Platform_SD_GetPath();
        FATFS *fs = (FATFS *)Platform_SD_GetFatFS();

        FRESULT FSRes = f_mount(fs, sd_path, 1);

        /* 挂载失败时尝试格式化 SD 卡 */
        /* If mount fails, try formatting the SD card */
        if (FSRes != FR_OK) {
            BYTE work[_MAX_SS];
            FSRes = f_mkfs(sd_path, FM_FAT32, 0, work, sizeof(work));

            if (FSRes == FR_OK) {
                printf("[xTaskInit]: Fatfs Format Succeed\r\n");
                addDevice("mnt", NULL, "SDcard", "FAT file system",
                          FILE_SYSTEM, DEVICE_ON, NULL);
                f_mount(fs, sd_path, 1);
            } else {
                printf("\n[xTaskInit]: SD Init Failed, please replace SD card\r\n");
            }
        } else {
            addDevice("mnt", fs, "SDcard", "FAT file system",
                      FILE_SYSTEM, DEVICE_ON, NULL);
            printf("[xTaskInit]: Fatfs Succeed\r\n");

            /* 修复: FatFs 未正确计算 database 字段 */
            /* Fix: FatFs may not correctly calculate database field */
#if _FS_EXFAT == 0
            if (fs->fs_type == FS_FAT32) {
                DWORD db = fs->fatbase + fs->n_fats * fs->fsize;
                if (fs->database != db) {
                    printf("[xTaskInit]: fix database %lu -> %lu\n",
                           (unsigned long)fs->database, (unsigned long)db);
                    fs->database = db;
                }
            }
#endif
        }
    }

    /* 初始化完成, 挂起本任务 / Init done, suspend this task */
    while (1) osDelay(1000);
}
