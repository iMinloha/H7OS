#include "touch_main.h"
#include "usbd_cdc_if.h"
#include "Core/DrT.h"
#include "memctl.h"
#include "fatfs.h"
#include <string.h>

extern FS_t currentFS;

void touch_main(int argc, char **argv){
    if (argc < 1) {
        USB_printf("Usage: touch <path> [content]\n");
        return;
    }

    char *content = (argc >= 2) ? argv[1] : NULL;
    uint16_t content_len = content ? (uint16_t)strlen(content) : 0;

    /* 如果在 SD 挂载点内, 直接在 SD 卡上创建文件 */
    char *sd_path = fs_to_sd_path(argv[0]);
    if (sd_path != NULL) {
        FIL file;
        FRESULT res = f_open(&file, sd_path, FA_CREATE_ALWAYS | FA_WRITE);
        if (res == FR_OK) {
            if (content_len > 0) {
                UINT bw;
                f_write(&file, content, content_len, &bw);
            }
            f_close(&file);
            USB_color_printf(LIGHT_GREEN, "touch: [SD] %s\n", sd_path);
        } else {
            USB_color_printf(LIGHT_RED, "touch: [SD] failed (%d)\n", res);
        }
        kernel_free(sd_path);
        return;
    }

    /* 否则在 RAMFS 中创建文件 */
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

    ram_touch(abs_path, content, content_len);
    kernel_free(abs_path);
}
