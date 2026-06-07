#include "cat_main.h"
#include "usbd_cdc_if.h"
#include "Core/DrT.h"
#include "memctl.h"
#include "fatfs.h"
#include <string.h>

extern FS_t currentFS;
extern FATFS SDFatFS;

/** Read a DrTFILE node's data buffer and print it */
static void cat_ramfs(const char *path) {
    char *p = kernel_alloc(strlen(path) + 1);
    strcpy(p, path);
    DrTNode_t dev = loadDevice(p);
    kernel_free(p);

    if (!dev || dev->type != DrTFILE) {
        USB_color_printf(LIGHT_RED, "cat: '%s' not a file\n", path);
        return;
    }
    if (dev->data) {
        USB_printf("%s", (char*)dev->data);
    }
}

/** Read a file from SD card via FatFs */
static void cat_sd(const char *sd_path) {
    FIL f;
    if (f_open(&f, sd_path, FA_READ) != FR_OK) {
        USB_color_printf(LIGHT_RED, "cat: [SD] '%s' not found\n", sd_path);
        return;
    }
    char buf[128];
    UINT br;
    while (f_read(&f, buf, sizeof(buf), &br) == FR_OK && br > 0) {
        buf[br] = '\0';
        USB_printf("%s", buf);
    }
    f_close(&f);
}

void cat_main(int argc, char **argv) {
    if (argc != 1) {
        USB_printf("Usage: cat <file>\n");
        return;
    }

    /* Check if inside SD mount point */
    char *sd_path = fs_to_sd_path(argv[0]);
    if (sd_path) {
        cat_sd(sd_path);
        kernel_free(sd_path);
        return;
    }

    /* Build absolute RAMFS path */
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
    cat_ramfs(abs_path);
    kernel_free(abs_path);
}
