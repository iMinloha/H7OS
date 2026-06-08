#include <stdio.h>
#include "RAMFS.h"
#include "usbd_cdc_if.h"
#include "fatfs.h"
#include <string.h>

#define ShowTab(level) for (int t = 0; t < level; t++) USB_printf("  ")

extern char SDPath[4];

static void dfs_sd_list(const char *sd_path, int level, int depth);

void dfs(FS_t node, int level, int depth) {
    /* Devices */
    DrTNode_t dev = node->node;
    for (int i = 0; i < node->node_count; i++) {
        ShowTab(level);
        USB_color_printf(LIGHT_BLUE, "%s %s\n",
            (i == node->node_count - 1) ? "`-" : "|-", dev->name);
        dev = dev->next;
    }

    /* Tasks */
    Task_t task = node->tasklist;
    while (task) {
        ShowTab(level);
        USB_color_printf(LIGHT_PURPLE, "%s %s\n",
            task->next ? "|-" : "`-", task->name);
        task = task->next;
    }

    /* RAMFS children */
    if (level <= depth) {
        FS_t child = node->child_next;
        while (child) {
            ShowTab(level);
            char *tag = child->sd_mount_path ? " -> [SD]" : "";
            USB_color_printf(LIGHT_GREEN, "%s %s%s\n",
                child->level_next ? "|-" : "`-", child->path, tag);
            dfs(child, level + 1, depth);
            child = child->level_next;
        }
    }

    /* SD mount */
    if (node->sd_mount_path && level < depth)
        dfs_sd_list(node->sd_mount_path, level + 1, depth);
}

static void dfs_sd_list(const char *sd_rel, int level, int depth) {
    char fatfs_path[256];
    strcpy(fatfs_path, SDPath);
    if (sd_rel[0]) strcat(fatfs_path, sd_rel);

    /* First pass: count items (dirs + files), skip dot entries */
    DIR dir;
    if (f_opendir(&dir, fatfs_path) != FR_OK) return;
    FILINFO f;
    int total = 0, dirs = 0, files = 0;
    while (f_readdir(&dir, &f) == FR_OK && f.fname[0]) {
        if (f.fname[0] == '.') continue;
        total++;
        if (f.fattrib & AM_DIR) dirs++; else files++;
    }
    f_closedir(&dir);

    /* Second pass: display all entries */
    if (f_opendir(&dir, fatfs_path) != FR_OK) return;
    int idx = 0;
    while (f_readdir(&dir, &f) == FR_OK && f.fname[0]) {
        if (f.fname[0] == '.') continue;
        idx++;
        int is_dir = (f.fattrib & AM_DIR) ? 1 : 0;
        int is_last = (idx >= total);
        ShowTab(level);
        USB_color_printf(is_dir ? LIGHT_GREEN : LIGHT_YELLOW, "%s %s%s\n",
            is_last ? "`-" : "|-", f.fname, is_dir ? "/" : "");
        if (is_dir && level < depth) {
            char sub[256];
            if (sd_rel[0]) sprintf(sub, "%s/%s", sd_rel, f.fname);
            else strcpy(sub, f.fname);
            dfs_sd_list(sub, level + 1, depth);
        }
    }
    f_closedir(&dir);
}

void DFS(FS_t node, int depth) { dfs(node, 0, depth); }
