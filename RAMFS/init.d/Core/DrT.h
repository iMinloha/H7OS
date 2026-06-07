#ifndef H7OS_DRT_H
#define H7OS_DRT_H

/**
 * DrT — Device Tree & RAM Filesystem
 *
 * Linux-inspired device tree: hardware devices, filesystem nodes,
 * process list, and shell command dispatch all live in this tree.
 */

#include "u_thread.h"
#include <stdio.h>
#include "Register.h"
#include "xTaskManager.h"
#include "fatfs.h"

/* ------------------------------------------------------------------ */
/* Device Types                                                       */
/* ------------------------------------------------------------------ */

enum DeviceType {
    DEVICE_TIMER,       /* Timer */
    DEVICE_BS,          /* Base device (e.g. CPU) */
    DEVICE_STORAGE,     /* Storage (QSPI, SD, eMMC) */
    DEVICE_DISPLAY,     /* Display (RGB, LVDS, HDMI) */
    DEVICE_INPUT,       /* Input device */
    DEVICE_SERIAL,      /* Serial (USART, UART, SPI, I2C, CAN) */
    DEVICE_TRANSPORT,   /* Transport (USB, ETH, WiFi) */
    DEVICE_VOTAGE,      /* Voltage (ADC, DAC) */
    DEVICE_TASK,        /* Task info (proc) */
    FILE_SYSTEM,        /* File system mount */
    DrTFILE,            /* Regular file */
    APP,                /* Application */
};

typedef enum DeviceType DeviceType_E;

enum DeviceStatus {
    DEVICE_OFF,
    DEVICE_ON,
    DEVICE_SUSPEND,
    DEVICE_ERROR,
    DEVICE_BUSY,
};

typedef enum DeviceStatus DeviceStatus_E;

/* ------------------------------------------------------------------ */
/* Core Structures                                                    */
/* ------------------------------------------------------------------ */

typedef struct FS*     FS_t;
typedef struct DrTNode* DrTNode_t;

/** Device node: hardware, file, or mount point */
struct DrTNode {
    void           *device;       /* HAL handle (e.g. &huart1) */
    DeviceStatus_E  status;
    DeviceType_E    type;
    char           *name;
    char           *description;
    void           *data;         /* File data or extra buffer */
    void           *fops;         /* bsp_file_ops_t* for I/O */
    Mutex_t         mutex;
    DrTNode_t       next;
    FS_t            parent;
};

/** Filesystem directory node */
struct FS {
    char       *path;             /* Directory name */
    DrTNode_t   node;             /* First child device */
    int         node_count;
    Task_t      tasklist;         /* Task list (for /proc) */
    uint8_t     allow_rm;         /* Removable flag */
    char       *sd_mount_path;    /* SD mount: base path on card */
    char       *sd_cd_path;       /* SD mount: current sub-path */
    FS_t        parent;
    FS_t        child_next;       /* First child directory */
    FS_t        level_next;       /* Next sibling */
};

/* ------------------------------------------------------------------ */
/* Command System                                                     */
/* ------------------------------------------------------------------ */

typedef void (*Comand_t)(int argc, char **argv);
typedef struct CMD* CMD_t;

struct CMD {
    char     *name;
    char     *description;
    char     *usage;
    Comand_t  cmd;
    CMD_t     next;
};

extern FS_t  RAM_FS;
extern CMD_t CMDList;
extern FS_t  currentFS;

FS_t fs_create_child(FS_t parent, char *path, uint8_t allow_rm);
FS_t getFSChild(FS_t parent, char *path);

void addCMD   (char *name, char *desc, char *usage, Comand_t cmd);
void execCMD  (char *command);
void helpCMD  (char *cmd);

/** Register a shell command */
#define CMD(name, description, usage, cmd) addCMD(name, description, usage, cmd)

/* ------------------------------------------------------------------ */
/* RAM Filesystem API                                                 */
/* ------------------------------------------------------------------ */

void   DrTInit(void);
void   addDevice(char *path, void *devPtr, char *name, char *desc,
                 DeviceType_E type, DeviceStatus_E status, void *driver);

/** Register a device and bind its file_ops in one step */
#define DEV_REGISTER(dir, hw, name, desc, type, fops_ptr) do { \
    addDevice(dir, hw, name, desc, type, DEVICE_ON, NULL);      \
    char _path[64]; sprintf(_path, "/%s/%s", dir, name);        \
    DrTNode_t _d = loadDevice(_path);                           \
    if (_d) _d->fops = (void*)(fops_ptr);                       \
} while(0)

FS_t       loadPath   (char *path);
DrTNode_t  loadDevice (char *path);
FS_t       ram_deep_mkdir(char *path);
void       ram_rm     (char *path);
void       ram_ls     (char *path);
FS_t       ram_cd     (char *path);
void       ram_pwd    (FS_t fs, char *path);
void       ram_touch  (char *full_path, void *data, uint16_t data_len);
char*      fs_to_sd_path(const char *rel_path);

/* ------------------------------------------------------------------ */
/* Device I/O (via file_ops)                                          */
/* ------------------------------------------------------------------ */

int dev_open (const char *path);
int dev_close(const char *path);
int dev_read (const char *path, uint8_t *buf, uint32_t len);
int dev_write(const char *path, const uint8_t *buf, uint32_t len);

/* ------------------------------------------------------------------ */
/* Process (Task) Management                                          */
/* ------------------------------------------------------------------ */

Task_t loadTask  (char *path);
void   addThread (Task_t task);
Task_t getThread (char *name);
Task_t getThreadByPID(uint8_t pid);
Task_t getTaskList(void);
Task_t getTaskByHandle(osThreadId handle);

#endif /* H7OS_DRT_H */
