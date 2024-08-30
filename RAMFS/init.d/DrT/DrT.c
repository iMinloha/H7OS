#include "DrT.h"
#include "memctl.h"
#include "usart.h"
#include "u_stdio.h"
#include "quadspi.h"
#include "fatfs.h"

// 串口设备指针
FS_t currentFS;

/**
 * @brief 添加子节点
 * @param parent 父节点
 * @param path 子节点路径
 */
void addFSChild(FS_t parent, char *path){
    // 创建子节点
    FS_t child = (FS_t) kernal_alloc(sizeof(struct FS));
    child->path = (char*) kernal_alloc(strlen(path) + 1);
    strcopy(child->path, path);

    child->node = (DrTNode_t) kernal_alloc(sizeof(struct DrTNode));
    child->node_count = 0;
    child->parent = parent;
    child->next = NULL; // 子文件夹
    child->child = NULL;    // 同层文件夹

    // 添加到父节点的next的child同层列表
    FS_t p = parent->next;
    if (p == NULL) {
        parent->next = child;
        return;
    }else{
        while(p->child != NULL) p = p->child;
        p->child = child;
    }
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
        p = p->child;
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
    DrTNode_t device = (DrTNode_t) kernal_alloc(sizeof(struct DrTNode));
    device->name = (char*) kernal_alloc(strlen(name) + 1);
    device->description = (char*) kernal_alloc(strlen(description) + 1);

    strcopy(device->name, name);
    strcopy(device->description, description);

    device->device = devicePtr;
    device->status = status;
    device->type = type;
    device->data = kernal_alloc(128);
    device->driver = driver;
    Mutex_t mutex = (Mutex_t) kernal_alloc(MUTEX_SIZE);
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
    // 创建根节点RootFS
    RAM_FS = (FS_t) kernal_alloc(sizeof(struct FS));
    CMDList = (CMD_t) kernal_alloc(sizeof(struct CMD));
    CMDList->next = NULL;
    RAM_FS->path = "/";

    RAM_FS->node = NULL;
    RAM_FS->node_count = 0;

    RAM_FS->parent = NULL;
    RAM_FS->next = NULL;
    RAM_FS->child = NULL;

    // 串口终端路径
    currentFS = RAM_FS;

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


    // 指令注册
    register_main();
}


/**
 * @brief 加载路径(获取节点)
 * @param path
 * @return
 */
FS_t loadPath(char* path) {
    FS_t node = RAM_FS;
    if (strcmp(path, "/") == 0) return node;
    else {
        if (path[0] == '/') path++;
        char *token = strtok(path, "/");
        while (token != NULL) {
            node = getFSChild(node, token);
            if (node == NULL) return NULL;
            token = strtok(NULL, "/");
        }
    }

    return node;
}

// DrT操作
// ====================================[指令操作]===================================

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

/**
 * @brief 显示目录
 * @param path
 */
void ram_ls(char* path){
    FS_t node = loadPath(path);
    if(node == NULL) return;

    FS_t temp = node->next;

    while(temp != NULL){
        // 输出node的子文件夹
        u_print("%s  ", temp->path);
        temp = temp->child;
    }

    if (node->node_count != 0) {
        DrTNode_t p = node->node->next;
        while(p != NULL){
            u_print("%s  ", p->name);
            p = p->next;
        }
    }

    u_print("\n");
}

/**
 * @brief 切换目录
 * @param path
 */
FS_t ram_cd(char* path){
    FS_t node = loadPath(path);
    if(node == NULL) return NULL;
    currentFS = node;
    return node;
}

void ram_pwd(FS_t fs, char* path){
    FS_t temp_node;
    // 递归显示路径
    if (fs == RAM_FS) {
        strcopy(path, "/");
        return;
    }else{
        temp_node = fs;
        ram_pwd(temp_node->parent, path);
        strconcat(path, temp_node->path);
        strconcat(path, "/");
    }

    path[strlen(path) - 1] = '\0';
}


// ============================[指令操作]===========================

// 添加指令
void addCMD(char* name, char* description, Comand_t cmd){
    CMD_t p = CMDList;
    while(p->next != NULL) p = p->next;
    CMD_t newCMD = (CMD_t) kernal_alloc(sizeof(struct CMD));


    newCMD->name = (char*) kernal_alloc(strlen(name) + 1);
    newCMD->description = (char*) kernal_alloc(strlen(description) + 1);
    strcopy(newCMD->name, name);
    strcopy(newCMD->description, description);

    newCMD->cmd = cmd;
    newCMD->next = NULL;
    p->next = newCMD;
}

// 执行指令
void execCMD(char* command){
    // command解析, command按空格分割保存到argv数组中
    char *argv[128] = {0};
    int argc = 0;
    char* token = strtok(command, " ");
    while(token != NULL){
        // 按空格分割
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    argc -= 1;

    // 查找指令
    CMD_t p = CMDList->next;
    while(p != NULL){
        if(strcmp(p->name, argv[0]) == 0){
            p->cmd(argc, &argv[1]);
            return;
        }
        p = p->next;
    }

    u_print("Command not found\n");
}