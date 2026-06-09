#include "run_main.h"
#include "Core/DrT.h"
#include "script.h"
#include "memctl.h"
#include "fatfs.h"
#include "usbd_cdc_if.h"
#include <string.h>
#include <stdio.h>

extern FS_t currentFS;
extern char SDPath[4];

void run_main(int argc, char **argv) {
    if (argc < 1) {
        USB_printf("Usage: run <script_file>\n");
        return;
    }

    char *src  = NULL;
    int   free_src = 0;

    /* ── 1. SD 卡 ───────────────────────────────────── */
    char *sd_rel = fs_to_sd_path(argv[0]);
    if (sd_rel) {
        FIL f;
        char fatfs_path[256];
        strcpy(fatfs_path, SDPath);
        if (sd_rel[0] == '/') strcat(fatfs_path, sd_rel);
        else                  { strcat(fatfs_path, "/"); strcat(fatfs_path, sd_rel); }

        if (f_open(&f, fatfs_path, FA_READ) == FR_OK) {
            UINT sz = f_size(&f);
            if (sz > 0 && sz < 8192) {
                src = kernel_alloc(sz + 1);
                UINT br;
                f_read(&f, src, sz, &br);
                src[br] = '\0';
                free_src = 1;
            }
            f_close(&f);
        }
        kernel_free(sd_rel);
        if (!src) { USB_color_printf(LIGHT_RED, "run: cannot read '%s'\n", argv[0]); return; }
    }

    /* ── 2. RAMFS ───────────────────────────────────── */
    if (!src) {
        char *p = kernel_alloc(strlen(argv[0]) + 1);
        strcpy(p, argv[0]);
        if (p[0] != '/') {
            char *abs = kernel_alloc(256);
            ram_pwd(currentFS, abs);
            uint32_t len = strlen(abs);
            if (len > 0 && abs[len - 1] != '/') { abs[len] = '/'; abs[len + 1] = '\0'; }
            strcat(abs, p);
            kernel_free(p);
            p = abs;
        }
        DrTNode_t dev = loadDevice(p);
        if (dev && dev->type == DrTFILE && dev->data) {
            src = (char*)dev->data;
            free_src = 0;
        }
        kernel_free(p);
        if (!src) { USB_color_printf(LIGHT_RED, "run: '%s' not found\n", argv[0]); return; }
    }

    /* ── 3. 异步启动 ────────────────────────────────── */
    osThreadId h = script_run_async(src, argv[0]);
    if (!h) {
        USB_color_printf(LIGHT_RED, "run: failed to start (max jobs reached?)\n");
    } else {
        USB_printf("started '%s'\n", argv[0]);
    }
    if (free_src) kernel_free(src);
}
