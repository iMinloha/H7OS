#include "Core/DrT.h"
#include "memctl.h"
#include "usbd_cdc_if.h"
#include "fatfs.h"
#include <string.h>
#include <stdio.h>

extern FS_t currentFS;

/* ── Directory tree helpers ──────────────────────────────── */

FS_t fs_create_child(FS_t parent, char* path, uint8_t allow_rm) {
    FS_t child = (FS_t)kernel_alloc(sizeof(struct FS));
    child->path = (char*)kernel_alloc(strlen(path) + 1);
    strcpy(child->path, path);
    child->node = NULL;
    child->node_count = 0;
    child->allow_rm = allow_rm;
    child->sd_mount_path = NULL;
    child->sd_cd_path = NULL;
    child->parent = parent;
    child->child_next = NULL;
    child->level_next = NULL;
    child->tasklist = NULL;
    FS_t p = parent->child_next;
    if (!p) {
        parent->child_next = child;
        return child;
    }
    while (p->level_next) p = p->level_next;
    p->level_next = child;
    return child;
}

void addPropertiesFSChild(FS_t parent, char* path) { fs_create_child(parent, path, 0); }
void addFSChild(FS_t parent, char* path) { fs_create_child(parent, path, 1); }

FS_t getFSChild(FS_t parent, char* path) {
    FS_t p = parent->child_next;
    while (p) {
        if (strcmp(p->path, path) == 0) return p;
        p = p->level_next;
    }
    return NULL;
}

FS_t loadPath(char* path) {
    FS_t node = RAM_FS;
    if (strcmp(path, "/") == 0) return node;
    if (strcmp(path, "../") == 0) return currentFS->parent;
    if (path[0] == '/') path++;
    char* token = strtok(path, "/");
    while (token) {
        node = getFSChild(node, token);
        if (!node) return NULL;
        token = strtok(NULL, "/");
    }
    return node;
}

/* ── mkdir ───────────────────────────────────────────────── */

FS_t ram_deep_mkdir(char* path) {
    FS_t node = (path[0] == '/' || currentFS == RAM_FS) ? RAM_FS : currentFS;
    if (strcmp(path, "/") == 0) return NULL;
    if (path[0] == '/') path++;
    char* token = strtok(path, "/");
    while (token) {
        FS_t next = getFSChild(node, token);
        if (!next) break;
        node = next;
        token = strtok(NULL, "/");
    }
    while (token) {
        addFSChild(node, token);
        node = getFSChild(node, token);
        token = strtok(NULL, "/");
    }
    return node;
}

/* ── rm ──────────────────────────────────────────────────── */

static void ram_rm_dir(FS_t node) {
    FS_t head = node->parent->child_next;
    FS_t p = head, prev = head;
    while (p) {
        if (p == node) break;
        prev = p;
        p = p->level_next;
    }
    if (p != node) return;
    if (head == node) node->parent->child_next = node->level_next;
    else prev->level_next = node->level_next;
    FS_t c = node->child_next;
    while (c) {
        FS_t n = c->level_next;
        ram_rm_dir(c);
        c = n;
    }
    kernel_free(node);
}

static void ram_rm_file(DrTNode_t device) {
    if (!device || device->type != DrTFILE) return;
    FS_t parent = device->parent;
    DrTNode_t p = parent->node, prev = parent->node;
    while (p) {
        if (p == device) {
            prev->next = p->next;
            kernel_free(p);
            return;
        }
        prev = p;
        p = p->next;
    }
}

void ram_rm(char* aim_dir) {
    char* p1 = kernel_alloc(strlen(aim_dir) + 1);
    char* p2 = kernel_alloc(strlen(aim_dir) + 1);
    strcpy(p1, aim_dir);
    strcpy(p2, aim_dir);
    FS_t node = loadPath(p1);
    DrTNode_t dev = loadDevice(p2);
    if (node) ram_rm_dir(node);
    else if (dev) ram_rm_file(dev);
    else USB_printf("rm: no such file or directory\n");
    kernel_free(p1);
    kernel_free(p2);
}

/* ── touch ───────────────────────────────────────────────── */

void ram_touch(char* full_path, void* data, uint16_t data_len) {
    char* path = kernel_alloc(strlen(full_path) + 1);
    strcpy(path, full_path);
    char* filename = strrchr(path, '/');
    FS_t parent;
    if (!filename) {
        parent = currentFS;
        filename = path;
    }
    else {
        *filename = '\0';
        filename++;
        parent = (*path) ? loadPath(path) : RAM_FS;
    }
    if (!parent) {
        USB_printf("touch: parent not found\n");
        kernel_free(path);
        return;
    }

    if (getFSChild(parent, filename)) {
        USB_printf("touch: '%s' exists\n", filename);
        kernel_free(path);
        return;
    }
    DrTNode_t d = parent->node;
    while (d) {
        if (strcmp(d->name, filename) == 0) {
            USB_printf("touch: '%s' exists\n", filename);
            kernel_free(path);
            return;
        }
        d = d->next;
    }

    DrTNode_t file = (DrTNode_t)kernel_alloc(sizeof(struct DrTNode));
    file->name = kernel_alloc(strlen(filename) + 1);
    strcpy(file->name, filename);
    file->description = kernel_alloc(8);
    strcpy(file->description, "file");
    file->type = DrTFILE;
    file->status = DEVICE_ON;
    file->device = NULL;
    file->fops = NULL;
    file->parent = parent;
    file->next = NULL;
    file->data = kernel_alloc(128);
    memset(file->data, 0, 128);
    if (data && data_len > 0) memoryCopy(file->data, data, data_len > 127 ? 127 : data_len);
    Mutex_t m = (Mutex_t)kernel_alloc(MUTEX_SIZE);
    mutex_init(m);
    file->mutex = m;
    DrTNode_t p = parent->node;
    if (!p) parent->node = file;
    else {
        while (p->next) p = p->next;
        p->next = file;
    }
    parent->node_count++;
    kernel_free(path);
}

/* ── ls ──────────────────────────────────────────────────── */

void ram_ls(char* path) {
    FS_t node = loadPath(path);
    if (!node) return;

    FS_t c = node->child_next;
    while (c) {
        USB_color_printf(LIGHT_GREEN, "%s  ", c->path);
        c = c->level_next;
    }

    DrTNode_t d = node->node;
    while (d) {
        USB_color_printf(LIGHT_BLUE, "%s  ", d->name);
        d = d->next;
    }

    if (node->sd_mount_path) {
        char* sd = node->sd_cd_path ? node->sd_cd_path : node->sd_mount_path;
        DIR dir;
        FILINFO f;
        if (f_opendir(&dir, sd) == FR_OK) {
            while (f_readdir(&dir, &f) == FR_OK && f.fname[0]) {
                if (f.fname[0] == '.') continue;
                USB_color_printf(f.fattrib & AM_DIR ? LIGHT_GREEN : LIGHT_YELLOW,
                                 f.fattrib & AM_DIR ? "%s/  " : "%s  ", f.fname);
            }
            f_closedir(&dir);
        }
    }

    Task_t t = node->tasklist;
    while (t) {
        USB_color_printf(LIGHT_PURPLE, "%s\t", t->name);
        t = t->next;
    }
    USB_printf("\n");
}

/* ── cd ──────────────────────────────────────────────────── */

FS_t ram_cd(char* path) {
    if (path[0] != '/' && currentFS->sd_mount_path) {
        char* base = currentFS->sd_cd_path ? currentFS->sd_cd_path : currentFS->sd_mount_path;
        if (strcmp(path, "..") == 0) {
            char* slash = strrchr(base, '/');
            if (slash) {
                *slash = '\0';
                if (strcmp(base, currentFS->sd_mount_path) == 0) {
                    if (currentFS->sd_cd_path) {
                        kernel_free(currentFS->sd_cd_path);
                        currentFS->sd_cd_path = NULL;
                    }
                    *slash = '/';
                }
                else {
                    if (currentFS->sd_cd_path) kernel_free(currentFS->sd_cd_path);
                    currentFS->sd_cd_path = kernel_alloc(strlen(base) + 1);
                    strcpy(currentFS->sd_cd_path, base);
                    *slash = '/';
                }
            }
            return currentFS;
        }
        char* new_sd = kernel_alloc(strlen(base) + strlen(path) + 2);
        sprintf(new_sd, "%s/%s", base, path);
        DIR d;
        if (f_opendir(&d, new_sd) == FR_OK) {
            f_closedir(&d);
            if (currentFS->sd_cd_path) kernel_free(currentFS->sd_cd_path);
            currentFS->sd_cd_path = new_sd;
            return currentFS;
        }
        kernel_free(new_sd);
        return currentFS;
    }
    if (path[0] == '/' && currentFS->sd_mount_path) {
        if (currentFS->sd_cd_path) {
            kernel_free(currentFS->sd_cd_path);
            currentFS->sd_cd_path = NULL;
        }
    }
    FS_t node = loadPath(path);
    if (!node) return NULL;
    currentFS = node;
    if (currentFS->sd_cd_path) {
        kernel_free(currentFS->sd_cd_path);
        currentFS->sd_cd_path = NULL;
    }
    return node;
}

/* ── pwd ─────────────────────────────────────────────────── */

static void pwd(FS_t fs, char* path) {
    if (fs == RAM_FS) {
        strcpy(path, "/");
        return;
    }
    pwd(fs->parent, path);
    strcat(path, fs->path);
    strcat(path, "/");
}

void ram_pwd(FS_t fs, char* path) {
    if (fs == RAM_FS) {
        strcpy(path, "/");
        return;
    }
    pwd(fs, path);
    path[strlen(path) - 1] = '\0';
}
