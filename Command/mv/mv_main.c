#include "mv_main.h"
#include "usbd_cdc_if.h"
#include "Core/DrT.h"
#include "memctl.h"
#include "fatfs.h"
#include <string.h>

extern FS_t currentFS;
extern char SDPath[4];

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

void mv_main(int argc, char **argv) {
    if (argc != 2) { USB_printf("Usage: mv <src> <dst>\n"); return; }

    char s[256], d[256];
    int ss = resolve(argv[0], s, sizeof(s));
    int ds = resolve(argv[1], d, sizeof(d));

    if (ss && ds) {
        /* SD → SD: atomic rename */
        if (f_rename(s, d) != FR_OK)
            USB_color_printf(LIGHT_RED, "mv: failed '%s' -> '%s'\n", argv[0], argv[1]);
    } else if (!ss && !ds) {
        /* RAMFS → RAMFS: copy data to new name, then remove old */
        DrTNode_t dev = loadDevice(s);
        if (!dev || dev->type != DrTFILE) {
            USB_color_printf(LIGHT_RED, "mv: '%s' not found\n", argv[0]);
            return;
        }
        /* Create destination with source data, then remove source */
        ram_touch(d, dev->data, dev->data ? strlen((char*)dev->data) : 0);
        ram_rm(s);
    } else {
        /* Cross-filesystem: copy then delete source */
        /* Use cat-like approach to read source, then write to dest, then remove source */
        char buf[256]; int len = 0;
        if (ss) {
            /* Read from SD */
            FIL fs; if (f_open(&fs, s, FA_READ) != FR_OK) goto mv_fail;
            UINT br;
            while (f_read(&fs, buf + len, 1, &br) == FR_OK && br > 0 && len < 255) len++;
            f_close(&fs);
            /* Write to RAMFS */
            ram_touch(d, buf, len);
            /* Delete SD source */
            f_unlink(s);
        } else {
            /* Read from RAMFS */
            DrTNode_t dev = loadDevice(s);
            if (!dev || dev->type != DrTFILE || !dev->data) goto mv_fail;
            len = strlen((char*)dev->data);
            memcpy(buf, dev->data, len);
            /* Write to SD */
            FIL fd;
            if (f_open(&fd, d, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
                UINT bw; f_write(&fd, buf, len, &bw);
                f_close(&fd);
            }
            /* Delete RAMFS source */
            ram_rm(s);
        }
    }
    return;
mv_fail:
    USB_color_printf(LIGHT_RED, "mv: failed '%s' -> '%s'\n", argv[0], argv[1]);
}
