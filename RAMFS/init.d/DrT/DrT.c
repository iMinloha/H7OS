#include "DrT.h"
#include "stm32h7xx_hal.h"
#include "memctl.h"
#include "usart.h"
#include "u_stdio.h"
#include "fatfs.h"
#include "quadspi.h"

/**
 * @brief 添加子节点
 * @param parent 父节点
 * @param path 子节点路径
 */
void addFSChild(FS_t parent, char *path){
    // 创建子节点
    FS_t child = (FS_t) ram_alloc(sizeof(struct FS));
    child->path = (char*) ram_alloc(strlen(path) + 1);
    strcopy(child->path, path);

    child->node = (DrTNode_t) ram_alloc(sizeof(struct DrTNode));
    child->node_count = 0;
    child->parent = parent;
    child->next = NULL;
    child->child = NULL;

    // 添加到父节点同层列表
    FS_t p = parent;
    while(p->next != NULL) p = p->next;
    p->next = child;
}

/**
 * @brief 获取子节点
 * @param parent 父节点
 * @param path 子节点路径
 * @return 子节点
 */
FS_t getFSChild(FS_t parent, char *path){
    FS_t p = parent->next;
    while(p != NULL){
        if(strcmp(p->path, path) == 0) return p;
        p = p->next;
    }
    return NULL;
}


// 添加设备
/**
 * @brief 添加设备
 * @param node 设备节点
 * @param devicePtr 设备指针(htim, huart, hspi)
 * @param name 设备名称(如TIM1, USART1, SPI1)
 * @param description 设备描述(如定时器1, 串口1, SPI1)
 * @param type 设备类型(如通信设备，时钟设备)
 * @param driver 设备驱动(一个函数指针)
 */
void addDevice(char *path, void* devicePtr, char *name, char *description, DeviceType_E type,
               DeviceStatus_E status, Func_t driver){
    FS_t node = getFSChild(RAM_FS, path);
    if (node == NULL) return;
    // 创建设备节点
    DrTNode_t device = (DrTNode_t) ram_alloc(sizeof(struct DrTNode));
    device->name = (char*) ram_alloc(strlen(name) + 1);
    device->description = (char*) ram_alloc(strlen(description) + 1);

    strcopy(device->name, name);
    strcopy(device->description, description);

    device->device = devicePtr;
    device->status = status;
    device->type = type;
    device->data = ram_alloc(128);
    device->driver = driver;
    Mutex_t mutex = (Mutex_t) ram_alloc(MUTEX_SIZE);
    mutex_init(mutex);
    device->mutex = mutex;
    device->parent = node;
    device->next = NULL;

    // 添加到设备链表
    DrTNode_t p = node->node;
    while(p->next != NULL) p = p->next;
    p->next = device;
    node->node_count++;
}

/**
 * @brief 初始化设备树
 */
void DrTInit(){
    RAM_FS = (FS_t) ram_alloc(sizeof(struct FS));
    RAM_FS->path = "/";

    RAM_FS->node = NULL;
    RAM_FS->node_count = 0;

    RAM_FS->parent = NULL;
    RAM_FS->next = NULL;
    RAM_FS->child = NULL;

    // 添加子路径
    addFSChild(RAM_FS, "dev");
    addFSChild(RAM_FS, "tmp");
    addFSChild(RAM_FS, "mnt");
    addFSChild(RAM_FS, "bin");
    addFSChild(RAM_FS, "usr");
    addFSChild(RAM_FS, "root");
    addFSChild(RAM_FS, "opt");
    addFSChild(RAM_FS, "etc");
    // 添加设备
    addDevice("dev", &huart1, "USART1", "Serial uart", DEVICE_SERIAL, DEVICE_BUSY, NULL);
    addDevice("mnt", &hsdram1, "SDMMC", "SD card", DEVICE_STORAGE, DEVICE_ON, NULL);
    addDevice("mnt", &SDFatFS, "FATFS", "FAT file system", FILE_SYSTEM, DEVICE_ON, NULL);
    addDevice("mnt", &hqspi, "QSPI", "Quad SPI", DEVICE_STORAGE, DEVICE_ON, NULL);
}

FS_t temp_node;

/**
 * @brief 获取设备
 * @return 设备节点
 */
 /*
void displayDevice(){
    FS_t node = RAM_FS;
    while (node->next != NULL){
        if(node->node_count != 0){
            u_print("path: %s \n",node->path);
            DrTNode_t p = node->node->next;
            while(p != NULL){
                u_print("====================\n");
                u_print("  name: %s \n", p->name);
                u_print("  description: %s \n", p->description);
                u_print("  status: %d \n", p->status);
                u_print("  type: %d \n", p->type);
                u_print("  device: %p \n", p->device);
                u_print("  data: %p \n", p->data);
                u_print("  driver: %p \n", p->driver);
                u_print("  mutex: %p \n", p->mutex);
                u_print("====================\n");
                p = p->next;
            }
        }
        node = node->next;
    }
}
  */

/**
 * @brief 加载路径
 * @param path
 * @return
 */
FS_t loadPath(char* path){
    FS_t node = RAM_FS;
    char* token = strtok(path, "/");
    while(token != NULL){
        node = getFSChild(node, token);
        if(node == NULL) return NULL;
        token = strtok(NULL, "/");
    }
    return node;
}

/**
 * @brief 创建目录
 * @param path
 * @param name
 */
void ram_mkdir(char* path, char* name){
    FS_t node = loadPath(path);
    if(node == NULL) return;
    addFSChild(node, name);
}

/**
 * @brief 创建文件
 * @param path
 * @param name
 */
void ram_mkfile(char* path, char* name){
    FS_t node = loadPath(path);
    if(node == NULL) return;
    addDevice(path, NULL, "file", "file", FILE, DEVICE_ON, NULL);
}

/**
 * @brief 删除目录
 * @param path
 * @param name
 */
void ram_rm(char* path, char *name){
    FS_t node = loadPath(path);
    if(node == NULL) return;
    FS_t p = node->next;
    FS_t pre = node;
    while(p != NULL){
        if(strcmp(p->path, name) == 0){
            pre->next = p->next;
            return;
        }
        pre = p;
        p = p->next;
    }
}

/**
 * @brief 读取文件
 * @param path
 * @param buf
 * @param size
 */
void ram_read(char* path, void* buf, int size){
    FS_t node = loadPath(path);
    if(node == NULL) return;
    DrTNode_t p = node->node->next;
    while(p != NULL){
        if(strcmp(p->name, "file") == 0){
            memoryCopy(buf, p->data, size);
            return;
        }
        p = p->next;
    }
}

/**
 * @brief 写入文件
 * @param path
 * @param buf
 * @param size
 */
void ram_write(char* path, void* buf, int size){
    FS_t node = loadPath(path);
    if(node == NULL) return;
    DrTNode_t p = node->node->next;
    while(p != NULL){
        if(strcmp(p->name, "file") == 0){
            memoryCopy(p->data, buf, size);
            return;
        }
        p = p->next;
    }
}