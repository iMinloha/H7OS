#include <stdio.h>
#include "RAMFS.h"
#include "usbd_cdc_if.h"
#include "fatfs.h"

#define ShowTab(level) for (int t = 0; t < level; t++) USB_printf("  ")

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

static void dfs_sd_list(const char *sd_path, int level, int depth) {
    DIR dir;
    if (f_opendir(&dir, sd_path) != FR_OK) return;
    FILINFO f;
    int count = 0;
    while (f_readdir(&dir, &f) == FR_OK && f.fname[0])
        if ((f.fattrib & AM_DIR) && f.fname[0] != '.') count++;
    f_closedir(&dir);

    if (f_opendir(&dir, sd_path) != FR_OK) return;
    int idx = 0;
    while (f_readdir(&dir, &f) == FR_OK && f.fname[0]) {
        if (!(f.fattrib & AM_DIR) || f.fname[0] == '.') continue;
        ShowTab(level);
        USB_color_printf(LIGHT_GREEN, "%s %s/\n",
            (++idx >= count) ? "`-" : "|-", f.fname);
        if (level < depth) {
            char sub[256];
            if (sd_path[0]) sprintf(sub, "%s/%s", sd_path, f.fname);
            else strcpy(sub, f.fname);
            dfs_sd_list(sub, level + 1, depth);
        }
    }
    f_closedir(&dir);
}

void DFS(FS_t node, int depth) { dfs(node, 0, depth); }
