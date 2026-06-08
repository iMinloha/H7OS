#include <stdio.h>
#include "mkdir_main.h"
#include "Core/DrT.h"
#include "usbd_cdc_if.h"
#include "memctl.h"
#include "fatfs.h"
#include "usart.h"

void mkdir_main(int argc, char **argv){
    if (argc != 1) {
        USB_printf("Usage: mkdir <path>\n");
        return;
    }

    /* 如果在 SD 挂载点内, 直接在 SD 卡上创建 */
    char *sd_path = fs_to_sd_path(argv[0]);
    if (sd_path != NULL) {
        FRESULT res = f_mkdir(sd_path);
        if (res == FR_OK)
            USB_color_printf(LIGHT_GREEN, "mkdir: [SD] %s\n", sd_path);
        else
            USB_color_printf(LIGHT_RED, "mkdir: [SD] failed (%d)\n", res);
        kernel_free(sd_path);
        return;
    }

    /* 否则在 RAMFS 中创建 */
    FS_t create = ram_deep_mkdir(argv[0]);
    if (create != NULL) {
        char *buf = kernel_alloc(48);
        ram_pwd(create, buf);
        USB_color_printf(LIGHT_GREEN, "mkdir: %s\n", buf);
        kernel_free(buf);
    } else {
        USB_color_printf(LIGHT_RED, "mkdir: failed to create %s\n", argv[0]);
    }
}
