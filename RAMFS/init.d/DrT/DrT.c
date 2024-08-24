#include "DrT.h"
#include "stm32h7xx_hal.h"
#include "memctl.h"
#include "usart.h"
#include "u_stdio.h"

void addFSChild(FS_t parent, char *path){
    // 创建子节点
    FS_t child = (FS_t) ram_alloc(sizeof(struct FS));
    child->path = (char*) ram_alloc(strlen(path) + 1);
    strcopy(child->path, path);

    child->node = (DrTNode_t) ram_alloc(sizeof(struct DrTNode));
    child->node_count = 0;
    child->parent = parent;
    child->next = NULL;
    child->Level = NULL;

    // 添加到父节点同层列表
    FS_t p = parent;
    while(p->Level != NULL) p = p->Level;
    p->Level = child;
}

FS_t getFSChild(FS_t parent, char *path){
    FS_t p = parent->Level;
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

void DrTInit(){
    RAM_FS = (FS_t) ram_alloc(sizeof(struct FS));
    RAM_FS->path = "/";

    RAM_FS->node = NULL;
    RAM_FS->node_count = 0;

    RAM_FS->parent = NULL;
    RAM_FS->next = NULL;
    RAM_FS->Level = NULL;

    // 添加子路径
    // TODO 这么添加都在ROOT的同级路径下了，应该是在ROOT下的子路径
    addFSChild(RAM_FS, "dev");
    addFSChild(RAM_FS, "tmp");
    addFSChild(RAM_FS, "mnt");
    addFSChild(RAM_FS, "bin");
    addFSChild(RAM_FS, "usr");
    addFSChild(RAM_FS, "root");
    addFSChild(RAM_FS, "opt");

    addDevice("dev", &huart1, "USART1", "Serial uart", DEVICE_SERIAL, DEVICE_BUSY, NULL);
}

FS_t temp_node;

void displayDevice(){
    FS_t node = RAM_FS;
    while (node->Level != NULL){
        u_print("path: %s \n",node->path);
        node = node->Level;
    }
}