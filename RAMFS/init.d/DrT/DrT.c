#include <string.h>
#include "DrT.h"
#include "memctl.h"
#include "usart.h"
#include "stdio.h"
#include "quadspi.h"
#include "fatfs.h"
#include "RAMFS.h"
#include "usbd_cdc_if.h"
#include "init.d/CPU/CS.h"

// 串口指针
FS_t currentFS;

void addPropertiesFSChild(FS_t parent, char *path){
    // 添加子文件系统(目录)
    FS_t child = (FS_t) kernel_alloc(sizeof(struct FS));
    child->path = (char*) kernel_alloc(strlen(path) + 1);
    strcpy(child->path, path);

    child->node = NULL;
    child->node_count = 0;
    child->allow_rm = 0;
    child->parent = parent;
    child->child_next = NULL; // 确保下一个节点是空
    child->level_next = NULL;
    child->tasklist = NULL;

    // 在父节点层节点下添加节点
    FS_t p = parent->child_next;
    if (p == NULL) {
        parent->child_next = child;
        return;
    }else{
        while(p->level_next != NULL) p = p->level_next;
        p->level_next = child;
    }
}


void addFSChild(FS_t parent, char *path){
    // 添加子文件系统(目录)
    FS_t child = (FS_t) kernel_alloc(sizeof(struct FS));
    child->path = (char*) kernel_alloc(strlen(path) + 1);
    strcpy(child->path, path);

    child->node = NULL;
    child->node_count = 0;
    child->allow_rm = 1;
    child->parent = parent;
    child->child_next = NULL; // 确保下一个节点是空
    child->level_next = NULL;
    child->tasklist = NULL;

    // 在父节点层节点下添加节点
    FS_t p = parent->child_next;
    if (p == NULL) {
        parent->child_next = child;
        return;
    }else{
        while(p->level_next != NULL) p = p->level_next;
        p->level_next = child;
    }
}


FS_t getFSChild(FS_t parent, char *path){
    FS_t p = parent->child_next;
    while(p != NULL){
        if(strcmp(p->path, path) == 0) return p;
        p = p->level_next;
    }
    return NULL;
}


void addDevice(char *path, void* devicePtr, char *name, char *description, DeviceType_E type,
               DeviceStatus_E status, Func_t driver){
    FS_t node = getFSChild(RAM_FS, path);
    if (node == NULL) return;
    // 添加设备到链表
    DrTNode_t device = (DrTNode_t) kernel_alloc(sizeof(struct DrTNode));
    device->name = (char*) kernel_alloc(strlen(name) + 1);
    device->description = (char*) kernel_alloc(strlen(description) + 1);

    strcpy(device->name, name);
    strcpy(device->description, description);

    device->device = devicePtr;
    device->status = status;
    device->type = type;

    device->data = kernel_alloc(128);
    device->driver = driver;

    Mutex_t mutex = (Mutex_t) kernel_alloc(MUTEX_SIZE);
    mutex_init(mutex);
    device->mutex = mutex;
    device->parent = node;
    device->next = NULL;

    // 添加设备到链表
    DrTNode_t p = node->node;
    if (p == NULL) {
        node->node = device;
        node->node_count++;
        return;
    }else{
        while(p->next != NULL) p = p->next;
        p->next = device;
        node->node_count++;
    }
}

void addThread(Task_t task){
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return;
    Task_t p = node->tasklist;
    if (p == NULL) {
        node->tasklist = task;
        return;
    }else{
        while(p->next != NULL) p = p->next;
        p->next = task;
    }
}


Task_t getThread(char* name){
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return NULL;
    Task_t p = node->tasklist;
    while(p != NULL){
        if(strcmp(p->name, name) == 0) return p;
        p = p->next;
    }
    return NULL;
}


Task_t getTaskList(){
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return NULL;
    return node->tasklist;
}


Task_t getTaskByHandle(osThreadId handle){
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return NULL;
    Task_t p = node->tasklist;
    while(p != NULL){
        if(p->handle == handle) return p;
        p = p->next;
    }
    return NULL;
}


Task_t getThreadByPID(uint8_t pid){
    FS_t node = getFSChild(RAM_FS, "proc");
    if (node == NULL) return NULL;
    Task_t p = node->tasklist;
    while(p != NULL){
        if(p->PID == pid) return p;
        p = p->next;
    }
    return NULL;
}


extern CPU_t CortexM7;

/**
 * @brief DrT设备树声明
 */
void DrTInit(){
    // Root文件系统
    RAM_FS = (FS_t) kernel_alloc(sizeof(struct FS));
    CMDList = (CMD_t) kernel_alloc(sizeof(struct CMD));
    CMDList->next = NULL;
    RAM_FS->path = "/";

    RAM_FS->node = NULL;
    RAM_FS->node_count = 0;

    RAM_FS->parent = NULL;
    RAM_FS->child_next = NULL;
    RAM_FS->level_next = NULL;
    RAM_FS->tasklist = NULL;

    // 串口设备指针
    currentFS = RAM_FS;

    // 添加系统文件夹
    addPropertiesFSChild(RAM_FS, "dev");  // 设备文件夹
    addPropertiesFSChild(RAM_FS, "mnt");  // 存储设备文件夹
    addPropertiesFSChild(RAM_FS, "usr");  // 用户文件夹(会自动保存在QSPI Flash中)
    addPropertiesFSChild(RAM_FS, "proc"); // 进程文件夹
    LOGGER("[DrT]: DrT Init\n");

    // 添加设备
    addDevice("dev", &CortexM7, "CPU", "Central Processing Unit", DEVICE_BS, DEVICE_BUSY, NULL);
    addDevice("dev", &huart1, "USART1", "Serial bus device", DEVICE_SERIAL, DEVICE_BUSY, NULL);

    // ָ指令注册
    register_main();
    LOGGER("[CMD]: Command Init\n");

    // 加载DrT
}

/***
 * @brief 加载路径,返回目录
 * @param path
 * @note 加载绝对路径获取FS_t与相对路径
 * */
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

/**
 * @brief 加载设备
 * @param path
 * @return
 */
DrTNode_t loadDevice(char* path_aim){
    char *path = (char*) kernel_alloc(strlen(path_aim) + 1);
    strcpy(path, path_aim);

    FS_t node;
    if (path[0] == '/') node = RAM_FS;
    else node = currentFS;

    if (strcmp(path, "/") == 0) return NULL;

    else {
        if (path[0] == '/') path++;
        char *token = strtok(path, "/");
        while (token != NULL) {
            FS_t tmp_node = getFSChild(node, token);
            if (tmp_node == NULL) break;
            token = strtok(NULL, "/");
            node = tmp_node;
        }


        if (strcmp(token, strtok(token, "/")) != 0){
            return NULL;
        } else {
            DrTNode_t p = node->node;
            while (p != NULL) {
                if (strcmp(p->name, token) == 0) return p;
                p = p->next;
            }
        }
    }
    return NULL;
}

/**
 * @brief 加载任务对象
 * @param path
 * @return
 */
Task_t loadTask(char* path){
    FS_t node;
    if (path[0] == '/') node = RAM_FS;
    else node = currentFS;

    if (strcmp(path, "/") == 0) return NULL;

    else {
        if (path[0] == '/') path++;
        char *token = strtok(path, "/");
        while (token != NULL) {
            FS_t tmp_node = getFSChild(node, token);
            if (tmp_node == NULL) break;
            token = strtok(NULL, "/");
            node = tmp_node;
        }

        // token不能包含任何的/
        if (strcmp(token, strtok(token, "/")) != 0) return NULL;
        else {
            Task_t p = node->tasklist;
            while (p != NULL) {
                if (strcmp(p->name, token) == 0) return p;
                p = p->next;
            }
        }
    }

    return NULL;
}

// DrT
// ====================================[RAM文件系统操作]===================================

// ram创建文件夹
void ram_mkdir(char* path){
    FS_t node = RAM_FS;
    if (strcmp(path, "/") == 0) return;
    else {
        if (path[0] == '/') path++;
        char *token = strtok(path, "/");
        while (token != NULL) {
            FS_t tmp_node;
            tmp_node = getFSChild(node, token);
            if (tmp_node == NULL) break;
            node = tmp_node;
            token = strtok(NULL, "/");
        }
        // token继续分割，在node下创建文件夹
        if (strtok(NULL, "/") == NULL) addPropertiesFSChild(node, token);
        else USB_printf("mkdir: %s: too many paths need to be created\n", path);
    }
}

FS_t ram_deep_mkdir(char *path){
    FS_t node;
    if(path[0] == '/' || currentFS == RAM_FS) node = RAM_FS;
    else node = currentFS;

    if (strcmp(path, "/") == 0) return NULL;
    else {
        if (path[0] == '/') path++;
        char *token = strtok(path, "/");
        while (token != NULL) {
            FS_t tmp_node;
            tmp_node = getFSChild(node, token);
            if (tmp_node == NULL) break;
            node = tmp_node;
            token = strtok(NULL, "/");
        }
        // token继续分割，在node下创建文件夹
        while(token != NULL){
            addFSChild(node, token);
            node = getFSChild(node, token);
            token = strtok(NULL, "/");
        }
        return node;
    }
}

// ram创建文件
void ram_mkfile(char* path, char* name){
    FS_t node = loadPath(path);
    if(node == NULL) return;
    addDevice(path, NULL, "file", "file", DrTFILE, DEVICE_ON, NULL);
}

// 删除FS_t对象
void ram_rm_dir(FS_t node){
    FS_t parent_child = node->parent->child_next;
    FS_t pFs = parent_child;
    FS_t preFS = parent_child;

    while (pFs != NULL){
        if (pFs == node) break;
        preFS = pFs;
        pFs = pFs->level_next;
    }

    // 没找到文件夹
    if(pFs != node) {
        USB_color_printf(LIGHT_RED, "rm: delete \"%s\" faild!\n", node->path);
        return;
    }

    // 判断是否是相同节点
    if (parent_child->level_next == NULL) node->parent->child_next = NULL;
    else if(preFS == pFs) node->parent->child_next = pFs->level_next;
    else preFS->level_next = pFs->level_next;

    FS_t aim_child_pointer = node->child_next;
    while (aim_child_pointer != NULL){
        FS_t aim_child = aim_child_pointer;
        aim_child_pointer = aim_child_pointer->level_next;
        ram_rm_dir(aim_child);
    }

    kernel_free(node);
}

// 未测试,没有对应文件
void ram_rm_file(DrTNode_t device){
    if(device == NULL) return;
    // device就是目标
    if (device->type == DrTFILE){
        // 执行删除操作
        FS_t parent = device->parent;
        DrTNode_t p = parent->node;
        DrTNode_t pre = parent->node;
        while(p != NULL){
            if(p == device){
                pre->next = p->next;
                kernel_free(p);
                return;
            }
            pre = p;
            p = p->next;
        }
    } else USB_color_printf(LIGHT_RED, "rm: delete \"%s\" faild!\n", device->name);
}


void ram_rm(char* aim_dir, Bool isDir){
    char *path = kernel_alloc(strlen(aim_dir) + 1);
    strcpy(path, aim_dir);
    FS_t node = loadPath(path);
    DrTNode_t device = loadDevice(path);
    if (node == NULL && device == NULL) {
        USB_printf("rm: no such file or folder, check it!\n");
        return;
    }
    else if(node != NULL && isDir == True) ram_rm_dir(node);
    else if(device != NULL && isDir == False) ram_rm_file(device);
}


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
 * @brief ram显示目录
 * @param path
 */
void ram_ls(char* path){
    FS_t node = loadPath(path);
    if(node == NULL) return;

    FS_t temp = node->child_next;

    while(temp != NULL){
        // node就是目标节点
        USB_color_printf(LIGHT_GREEN, "%s  ", temp->path);
        temp = temp->level_next;
    }

    if (node->node_count != 0) {
        DrTNode_t p = node->node;
        while(p != NULL){
            USB_color_printf(LIGHT_BLUE, "%s  ", p->name);
            p = p->next;
        }
    }

    if(node->tasklist != NULL){
        Task_t p = node->tasklist;
        while(p != NULL){
            USB_color_printf(LIGHT_PURPLE, "%s\t", p->name);
            p = p->next;
        }
    }

    USB_printf("\n");
}

/**
 * @brief 切换串口指针到指定目录
 * @param path
 */
FS_t ram_cd(char* path){
    FS_t node = loadPath(path);
    if(node == NULL) return NULL;
    currentFS = node;
    return node;
}

void pwd(FS_t fs, char* path){
    FS_t temp_node;
    if (fs == RAM_FS) {
        strcpy(path, "/");
        return;
    }else{
        temp_node = fs;
        pwd(temp_node->parent, path);
        strcat(path, temp_node->path);
        strcat(path, "/");
    }

    path[strlen(path)] = '\0';

}

void ram_pwd(FS_t fs, char* path){
    if (fs == RAM_FS) {
        strcpy(path, "/");
        return;
    }
    pwd(fs, path);
    path[strlen(path) - 1] = '\0';
}


// ============================[ָ指令系统]===========================
void addCMD(char* name, char* description, char* usage, Comand_t cmd){
    CMD_t p = CMDList;
    while(p->next != NULL) p = p->next;
    CMD_t newCMD = (CMD_t) kernel_alloc(sizeof(struct CMD));


    newCMD->name = (char*) kernel_alloc(strlen(name) + 1);
    newCMD->description = (char*) kernel_alloc(strlen(description) + 1);
    newCMD->usage = (char*) kernel_alloc(strlen(usage) + 1);
    strcpy(newCMD->name, name);
    strcpy(newCMD->description, description);
    strcpy(newCMD->usage, usage);

    newCMD->cmd = cmd;
    newCMD->next = NULL;
    p->next = newCMD;
}

// 执行指令
void execCMD(char* command_rel){
    // 使用指令前需要先声明一个内存空间用于保护指令所处空间的安全
    char* command = (char*) kernel_alloc(strlen(command_rel) + 1);
    strcpy(command, command_rel);

    // 分割指令
    char *argv[128] = {0};
    int argc = 0;
    char* token = strtok(command, " ");
    while(token != NULL){
        // 遍历并分割
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    argc -= 1;


    // 在指令集中查找指令，如果找到则执行
    CMD_t p = CMDList->next;
    while(p != NULL){
        if(strcmp(p->name, argv[0]) == 0){
            p->cmd(argc, &argv[1]);
            return;
        }
        p = p->next;
    }

    USB_printf("Command not found\n");
}

void helpCMD(char *cmd){
    char buf[128];
    memoryCopy(buf, cmd, strlen(cmd) + 1);
    CMD_t p = CMDList->next;
    USB_printf("Command\t\tDescription\t\tUsage\n");
    if(buf[0] == '\0') {
        while(p != NULL){
            USB_printf("%s\t\t%s\t\t%s\n", p->name, p->description, p->usage);
            p = p->next;
        }
    }else{
        while(p != NULL){
            if(strcmp(p->name, cmd) == 0){
                USB_printf("%s\t\t%s\t\t%s\n", p->name, p->description, p->usage);
                return;
            }
            p = p->next;
        }

    }
}