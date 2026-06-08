#include "rm_main.h"
#include "usbd_cdc_if.h"
#include "Core/DrT.h"
#include "memctl.h"
#include "fatfs.h"
#include <string.h>

extern FS_t currentFS;

void rm_main(int argc, char *argv[]){
    if (argc != 1) {
        USB_printf("Usage: rm <path>\n");
        return;
    }

    /* 如果在 SD 挂载点内, 直接在 SD 卡上删除 */
    char *sd_path = fs_to_sd_path(argv[0]);
    if (sd_path != NULL) {
        FRESULT res = f_unlink(sd_path);
        if (res != FR_OK) {
            /* f_unlink 失败, 可能是非空目录, 尝试确认是否目录存在 */
            DIR test;
            if (f_opendir(&test, sd_path) == FR_OK) {
                f_closedir(&test);
                res = f_unlink(sd_path);
                if (res != FR_OK) {
                    USB_color_printf(LIGHT_RED, "rm: cannot remove '%s': directory not empty\n", sd_path);
                }
            } else {
                USB_color_printf(LIGHT_RED, "rm: '%s' not found\n", sd_path);
            }
        }
        kernel_free(sd_path);
        return;
    }

    /* 否则在 RAMFS 中删除 */
    char *abs_path = kernel_alloc(256);
    if (argv[0][0] == '/') {
        strcpy(abs_path, argv[0]);
    } else {
        ram_pwd(currentFS, abs_path);
        uint32_t len = strlen(abs_path);
        if (len > 0 && abs_path[len - 1] != '/') {
            abs_path[len] = '/';
            abs_path[len + 1] = '\0';
        }
        strcat(abs_path, argv[0]);
    }

    ram_rm(abs_path);
    kernel_free(abs_path);
}
