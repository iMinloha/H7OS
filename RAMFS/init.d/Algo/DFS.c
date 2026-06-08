#include <stdio.h>
#include "RAMFS.h"
#include "usbd_cdc_if.h"
#include "fatfs.h"
#include <string.h>

extern char SDPath[4];

static void dfs_sd_list(const char *sd_rel, const char *prefix, int depth);

/* Draw a tree node: prefix + branch + name */
static void draw_node(const char *prefix, int is_last, int is_dir, const char *name) {
    USB_printf("%s", prefix);
    USB_printf(is_last ? "`-" : "|-");
    USB_color_printf(is_dir ? LIGHT_GREEN : LIGHT_YELLOW, " %s%s\n",
                     name, is_dir ? "/" : "");
}

/* ── RAMFS tree ──────────────────────────────────────────── */
static void dfs(FS_t node, const char *prefix, int depth) {
    /* Count total visible children (devices + dirs + tasks + SD) */
    int devs = node->node_count;
    int tasks = 0;
    Task_t t = node->tasklist; while (t) { tasks++; t = t->next; }
    int dirs = 0;
    FS_t c = node->child_next; while (c) { dirs++; c = c->level_next; }
    int has_sd = (node->sd_mount_path != NULL) ? 1 : 0;
    int total = devs + tasks + dirs + has_sd;
    int idx = 0;

    /* Devices */
    DrTNode_t dev = node->node;
    while (dev) {
        idx++;
        draw_node(prefix, idx >= total, 0, dev->name);
        dev = dev->next;
    }

    /* Tasks */
    t = node->tasklist;
    while (t) {
        idx++;
        draw_node(prefix, idx >= total, 0, t->name);
        t = t->next;
    }

    /* RAMFS children */
    if (depth > 0) {
        c = node->child_next;
        while (c) {
            idx++;
            char tag[16] = "";
            if (c->sd_mount_path) strcpy(tag, " -> [SD]");
            char name_buf[64];
            snprintf(name_buf, sizeof(name_buf), "%s%s", c->path, tag);
            draw_node(prefix, idx >= total, 1, name_buf);

            /* Build child prefix: add "|   " or "    " */
            char child_prefix[256];
            snprintf(child_prefix, sizeof(child_prefix), "%s%s",
                     prefix, idx >= total ? "    " : "|   ");
            dfs(c, child_prefix, depth - 1);
            c = c->level_next;
        }
    }

    /* SD mount content */
    if (has_sd && depth > 0) {
        char child_prefix[256];
        snprintf(child_prefix, sizeof(child_prefix), "%s    ", prefix);
        dfs_sd_list(node->sd_mount_path, child_prefix, depth - 1);
    }
}

/* ── SD card tree ────────────────────────────────────────── */
static void dfs_sd_list(const char *sd_rel, const char *prefix, int depth) {
    char fatfs_path[256];
    strcpy(fatfs_path, SDPath);
    if (sd_rel[0]) strcat(fatfs_path, sd_rel);

    /* First pass: count entries, skip dot files */
    DIR dir;
    if (f_opendir(&dir, fatfs_path) != FR_OK) return;
    FILINFO f;
    int total = 0;
    while (f_readdir(&dir, &f) == FR_OK && f.fname[0]) {
        if (f.fname[0] != '.') total++;
    }
    f_closedir(&dir);

    /* Second pass: display entries */
    if (f_opendir(&dir, fatfs_path) != FR_OK) return;
    int idx = 0;
    while (f_readdir(&dir, &f) == FR_OK && f.fname[0]) {
        if (f.fname[0] == '.') continue;
        idx++;
        int is_dir = (f.fattrib & AM_DIR) ? 1 : 0;
        int is_last = (idx >= total);
        draw_node(prefix, is_last, is_dir, f.fname);

        if (is_dir && depth > 0) {
            char sub_rel[256];
            if (sd_rel[0]) snprintf(sub_rel, sizeof(sub_rel), "%s/%s", sd_rel, f.fname);
            else strncpy(sub_rel, f.fname, sizeof(sub_rel));
            char child_prefix[256];
            snprintf(child_prefix, sizeof(child_prefix), "%s%s",
                     prefix, is_last ? "    " : "|   ");
            dfs_sd_list(sub_rel, child_prefix, depth - 1);
        }
    }
    f_closedir(&dir);
}

void DFS(FS_t node, int depth) {
    dfs(node, "", depth);
}
