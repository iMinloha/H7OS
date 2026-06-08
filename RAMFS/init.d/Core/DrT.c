/**
 * @file    DrT.c
 * @brief   DrT 设备树 + RAMFS 初始化
 *          DrT device tree + RAMFS initialization
 *
 * 只通过 platform.h 访问硬件, 不直接引用 HAL 或 BSP 头文件。
 * Only accesses hardware through platform.h, no direct HAL/BSP includes.
 */

#include "Core/DrT.h"
#include "memctl.h"
#include "platform.h"
#include <string.h>
#include "Periph/cpu.h"

FS_t RAM_FS;
CMD_t CMDList;
FS_t currentFS;
extern CPU_t CortexM7;

void DrTInit(void) {
    RAM_FS = (FS_t)kernel_alloc(sizeof(struct FS));
    CMDList = (CMD_t)kernel_alloc(sizeof(struct CMD));
    CMDList->next = NULL;
    RAM_FS->path = "/";
    RAM_FS->node = NULL;
    RAM_FS->node_count = 0;
    RAM_FS->parent = NULL;
    RAM_FS->child_next = NULL;
    RAM_FS->level_next = NULL;
    RAM_FS->sd_mount_path = NULL;
    RAM_FS->sd_cd_path = NULL;
    RAM_FS->tasklist = NULL;
    currentFS = RAM_FS;

    fs_create_child(RAM_FS, "dev", 0);
    fs_create_child(RAM_FS, "mnt", 0);
    fs_create_child(RAM_FS, "usr", 0);
    fs_create_child(RAM_FS, "proc", 0);
    fs_create_child(RAM_FS, "bin", 0);

    /* Device subdirectories */
    FS_t dev = getFSChild(RAM_FS, "dev");
    fs_create_child(dev, "cpu", 0);
    fs_create_child(dev, "serial", 0);
    fs_create_child(dev, "gpio", 0);
    fs_create_child(dev, "pwm", 0);
    fs_create_child(dev, "adc", 0);
    fs_create_child(dev, "i2c", 0);

    /* 注册所有板载外设到 DrT (通过 Platform API) */
    /* Register all onboard peripherals to DrT (via Platform API) */
    Platform_DevicesInit();

    register_main();
}
