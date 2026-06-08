#include "Core/DrT.h"
#include "memctl.h"
#include <string.h>
#include <stdio.h>

#include "bsp_file_ops.h"

extern FS_t currentFS;

void addDevice(char* path, void* devicePtr, char* name, char* description, DeviceType_E type,
               DeviceStatus_E status, void* driver) {
    /* Walk multi-level path (e.g. "dev/cpu") to find target FS directory */
    FS_t node = RAM_FS;
    char* path_dup = (char*)kernel_alloc(strlen(path) + 1);
    strcpy(path_dup, path);
    char* token = strtok(path_dup, "/");
    while (token) {
        FS_t next = getFSChild(node, token);
        if (!next) {
            kernel_free(path_dup);
            return;
        }
        node = next;
        token = strtok(NULL, "/");
    }
    kernel_free(path_dup);

    /* ── 防重复: 检查同名设备是否已存在 ────────────────────── */
    /*    Dedup: check if device with same name already exists   */
    {
        DrTNode_t exist = node->node;
        while (exist) {
            if (exist->name && strcmp(exist->name, name) == 0) {
                /* 设备已存在, 静默跳过 (防止重复注册) */
                /* Device already exists, silently skip (prevents duplicates) */
                return;
            }
            exist = exist->next;
        }
    }

    DrTNode_t device = (DrTNode_t)kernel_alloc(sizeof(struct DrTNode));
    device->name = (char*)kernel_alloc(strlen(name) + 1);
    device->description = (char*)kernel_alloc(strlen(description) + 1);
    strcpy(device->name, name);
    strcpy(device->description, description);
    device->device = devicePtr;
    device->status = status;
    device->type = type;
    device->data = kernel_alloc(128);
    device->fops = driver;
    (void)driver; /* used above */

    Mutex_t mutex = (Mutex_t)kernel_alloc(MUTEX_SIZE);
    mutex_init(mutex);
    device->mutex = mutex;
    device->parent = node;
    device->next = NULL;

    DrTNode_t p = node->node;
    if (p == NULL) {
        node->node = device;
        node->node_count++;
        return;
    }
    else {
        while (p->next != NULL) p = p->next;
        p->next = device;
        node->node_count++;
    }
}

DrTNode_t loadDevice(char* path_aim) {
    char* orig = (char*)kernel_alloc(strlen(path_aim) + 1);
    strcpy(orig, path_aim);
    char* path = orig;

    FS_t node = (path[0] == '/') ? RAM_FS : currentFS;
    if (strcmp(path, "/") == 0) {
        kernel_free(orig);
        return NULL;
    }
    if (path[0] == '/') path++;

    char* token = strtok(path, "/");
    while (token) {
        FS_t next = getFSChild(node, token);
        if (!next) break;
        node = next;
        token = strtok(NULL, "/");
    }

    if (token && node && node->node) {
        DrTNode_t p = node->node;
        while (p) {
            if (p->name && strcmp(p->name, token) == 0) {
                kernel_free(orig);
                return p;
            }
            p = p->next;
        }
    }

    kernel_free(orig);
    return NULL;
}

/* ============================[ 通用设备 I/O ]=========================== */

static bsp_file_ops_t* dev_get_fops(const char* path) {
    char* buf = kernel_alloc(strlen(path) + 1);
    strcpy(buf, path);
    DrTNode_t dev = loadDevice(buf);
    kernel_free(buf);
    if (!dev || !dev->fops) return NULL;
    return (bsp_file_ops_t*)dev->fops;
}

int dev_open(const char* path) {
    char* buf = kernel_alloc(strlen(path) + 1);
    strcpy(buf, path);
    DrTNode_t dev = loadDevice(buf);
    kernel_free(buf);
    if (!dev || !dev->fops) return -1;
    bsp_file_ops_t* f = (bsp_file_ops_t*)dev->fops;
    int ret = f->open ? f->open(dev->device) : 0;
    if (ret == 0) dev->status = DEVICE_BUSY;
    return ret;
}

int dev_close(const char* path) {
    char* buf = kernel_alloc(strlen(path) + 1);
    strcpy(buf, path);
    DrTNode_t dev = loadDevice(buf);
    kernel_free(buf);
    if (!dev || !dev->fops) return -1;
    bsp_file_ops_t* f = (bsp_file_ops_t*)dev->fops;
    int ret = f->close ? f->close(dev->device) : 0;
    dev->status = DEVICE_ON;
    return ret;
}

int dev_read(const char* path, uint8_t* buf, uint32_t len) {
    char* p = kernel_alloc(strlen(path) + 1);
    strcpy(p, path);
    DrTNode_t dev = loadDevice(p);
    kernel_free(p);
    if (!dev || !dev->fops) return -1;
    bsp_file_ops_t* f = (bsp_file_ops_t*)dev->fops;
    return f->read ? f->read(dev->device, buf, len) : -1;
}

int dev_write(const char* path, const uint8_t* buf, uint32_t len) {
    char* p = kernel_alloc(strlen(path) + 1);
    strcpy(p, path);
    DrTNode_t dev = loadDevice(p);
    kernel_free(p);
    if (!dev || !dev->fops) return -1;
    bsp_file_ops_t* f = (bsp_file_ops_t*)dev->fops;
    return f->write ? f->write(dev->device, buf, len) : -1;
}
