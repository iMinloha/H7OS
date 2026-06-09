#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree_main.h"
#include "RAMFS.h"
#include "Core/DrT.h"
#include "usbd_cdc_if.h"
#include "memctl.h"
#include "fatfs.h"

extern FS_t currentFS;
extern char SDPath[4];

void tree_main(int argc, char **argv){
    USB_printf("\n");

    int   depth  = 1;
    FS_t  target = currentFS;
    char *sd_sub = NULL;       /* SD sub-path override (kernel_alloc'd) */

    /* ── Parse arguments ─────────────────────────────────── */
    int path_arg = 0;          /* which argv index is the path */

    if (argc >= 1) {
        /* Check if argv[0] is a pure integer (depth-only mode, backward compat) */
        int is_num = 1;
        if (argv[0][0] != '\0') {
            for (char *p = argv[0]; *p; p++) {
                if (*p < '0' || *p > '9') { is_num = 0; break; }
            }
        } else {
            is_num = 0;
        }

        if (is_num) {
            depth = atoi(argv[0]);
        } else {
            path_arg = 0;
            if (argc >= 2) depth = atoi(argv[1]);
        }
    }

    /* ── Resolve path (if given) ──────────────────────────── */
    if (argc >= 1 && path_arg == 0 && argv[0][0] != '\0') {
        /* Try direct RAMFS resolution */
        FS_t node = loadPath(argv[0]);
        if (node) {
            target = node;
        } else {
            /* Path may go into an SD mount — walk component by component.
             * Find the deepest RAMFS ancestor, then validate SD sub-path. */
            char *path = kernel_alloc(strlen(argv[0]) + 1);
            strcpy(path, argv[0]);
            if (path[0] == '/') path++;  /* skip leading '/' */

            FS_t ram_node = RAM_FS;
            char *saveptr;
            char *token = strtok_r(path, "/", &saveptr);
            char *last_ramfs = NULL;   /* last token that resolved in RAMFS */
            FS_t  last_fs    = ram_node;

            while (token) {
                FS_t child = getFSChild(ram_node, token);
                if (child) {
                    ram_node = child;
                    last_fs  = child;
                    last_ramfs = token;
                    token = strtok_r(NULL, "/", &saveptr);
                } else {
                    break;  /* rest is SD sub-path */
                }
            }

            /* Build remaining SD sub-path */
            if (token && last_fs->sd_mount_path) {
                char *base = last_fs->sd_cd_path
                           ? last_fs->sd_cd_path
                           : last_fs->sd_mount_path;
                int sd_len = strlen(base) + 1;  /* +1 for '/' or '\0' */
                char *remaining = saveptr ? saveptr : token;
                if (remaining && remaining[0]) sd_len += 1 + strlen(remaining);

                char *test_sd = kernel_alloc(sd_len + 1);
                strcpy(test_sd, base);
                if (token) {
                    if (base[0]) strcat(test_sd, "/");
                    strcat(test_sd, token);
                    /* also append any further components */
                    token = strtok_r(NULL, "/", &saveptr);
                    while (token) {
                        strcat(test_sd, "/");
                        strcat(test_sd, token);
                        token = strtok_r(NULL, "/", &saveptr);
                    }
                }

                /* Validate on SD card */
                char fatfs_path[256];
                strcpy(fatfs_path, SDPath);
                if (test_sd[0]) strcat(fatfs_path, test_sd);

                DIR test_dir;
                if (f_opendir(&test_dir, fatfs_path) == FR_OK) {
                    f_closedir(&test_dir);
                    target = last_fs;
                    sd_sub = test_sd;
                } else {
                    USB_color_printf(LIGHT_RED, "tree: path not found on SD: %s\n", argv[0]);
                    kernel_free(test_sd);
                    kernel_free(path);
                    return;
                }
            } else {
                USB_color_printf(LIGHT_RED, "tree: path not found: %s\n", argv[0]);
                kernel_free(path);
                return;
            }
            kernel_free(path);
        }
    }

    /* ── Execute tree ─────────────────────────────────────── */
    /* Save/restore sd_cd_path if we're overriding it */
    char *saved_sd_cd = NULL;
    if (sd_sub && target->sd_cd_path) {
        saved_sd_cd = target->sd_cd_path;
    }

    if (sd_sub) {
        target->sd_cd_path = sd_sub;
    }

    DFS(target, depth);

    /* Restore */
    if (sd_sub) {
        target->sd_cd_path = saved_sd_cd;
        kernel_free(sd_sub);
    }
}
