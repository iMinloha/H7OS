#include "cp_main.h"
#include "usbd_cdc_if.h"
#include "Core/DrT.h"
#include "memctl.h"
#include "fatfs.h"
#include <string.h>

extern FS_t currentFS;
extern char SDPath[4];

/* Build absolute path for RAMFS or SD. Returns 1 if SD, 0 if RAMFS. */
static int resolve(const char *input, char *out, int out_sz) {
    /* Absolute path → always RAMFS */
    if (input[0] == '/') {
        strncpy(out, input, out_sz);
        return 0;
    }
    /* Relative path → check SD mount first */
    char *sd = fs_to_sd_path(input);
    if (sd) {
        strcpy(out, SDPath);
        if (sd[0] == '/') strcat(out, sd + 1);
        else strcat(out, sd);
        kernel_free(sd);
        return 1;
    }
    /* Relative path in RAMFS */
    ram_pwd(currentFS, out);
    int len = strlen(out);
    if (len > 0 && out[len - 1] != '/') strcat(out, "/");
    strncat(out, input, out_sz - strlen(out) - 1);
    return 0;
}

void cp_main(int argc, char **argv) {
    if (argc != 2) { USB_printf("Usage: cp <src> <dst>\n"); return; }

    char s[256], d[256];
    int ss = resolve(argv[0], s, sizeof(s));
    int ds = resolve(argv[1], d, sizeof(d));

    if (ss && ds) {
        /* SD → SD */
        FIL fs, fd;
        if (f_open(&fs, s, FA_READ) != FR_OK) goto fail;
        if (f_open(&fd, d, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) { f_close(&fs); goto fail; }
        char buf[256]; UINT br, bw;
        while (f_read(&fs, buf, sizeof(buf), &br) == FR_OK && br > 0) {
            if (f_write(&fd, buf, br, &bw) != FR_OK || bw < br) break;
        }
        f_close(&fs); f_close(&fd);
    } else if (!ss && ds) {
        /* RAMFS → SD */
        DrTNode_t dev = loadDevice(s);
        if (!dev || dev->type != DrTFILE || !dev->data) goto fail;
        FIL fd;
        if (f_open(&fd, d, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) goto fail;
        int len = strlen((char*)dev->data);
        UINT bw; f_write(&fd, dev->data, len, &bw);
        f_close(&fd);
    } else if (ss && !ds) {
        /* SD → RAMFS */
        FIL fs;
        if (f_open(&fs, s, FA_READ) != FR_OK) goto fail;
        char tmp[128]; UINT br, total = 0;
        while (f_read(&fs, tmp + total, 1, &br) == FR_OK && br > 0 && total < 127) total++;
        tmp[total] = '\0';
        f_close(&fs);
        ram_touch(d, tmp, total);
    } else {
        /* RAMFS → RAMFS */
        DrTNode_t dev = loadDevice(s);
        if (!dev || dev->type != DrTFILE || !dev->data) goto fail;
        ram_touch(d, dev->data, strlen((char*)dev->data));
    }
    return;
fail:
    USB_color_printf(LIGHT_RED, "cp: failed '%s' -> '%s'\n", argv[0], argv[1]);
}
