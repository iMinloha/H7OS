#include "test.h"
#include "u_stdio.h"
#include "cmsis_os.h"
#include "quadspi.h"
#include "timer.h"
#include "fatfs.h"
#include "memctl.h"

FRESULT FSRes;

void testFuncInit(){
    FATFS_LinkDriver(&SD_Driver, SDPath);
    FSRes = f_mount(&SDFatFS,SDPath,1);
    BYTE work[_MAX_SS];
    if (FSRes != FR_OK){
        FSRes = f_mkfs(SDPath, FM_FAT32, 0, work, sizeof work);
        if (FSRes == FR_OK) u_print("SD card init succeed\r\n");
        else u_print("Init Faild, please replace SD card\r\n");
    }

    if(QSPI_W25Qxx_BlockErase_32K(0) != QSPI_W25Qxx_OK)
        u_print("Erase Failed\n");

}

void testFunc(){
    // kernel_test();
    uint8_t tt[10] = {0};
    void *p = ram_alloc(10);
    u_print("inside ram: %p\n", tt);
    u_print("sdram: %p\n", p);
    ram_free(p);
    osDelay(1000);
}