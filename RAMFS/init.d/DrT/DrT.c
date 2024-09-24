#include <string.h>
#include "DrT.h"
#include "memctl.h"
#include "usart.h"
#include "stdio.h"
#include "quadspi.h"
#include "fatfs.h"
#include "RAMFS.h"

// �����豸ָ��(����ָ��ǰ�����ն�����λ��)
FS_t currentFS;

/**
 * @brief �����ӽڵ�
 * @param parent ���ڵ�
 * @param path �ӽڵ�·��
 */
void addFSChild(FS_t parent, char *path){
    // �����ӽڵ�
    FS_t child = (FS_t) kernel_alloc(sizeof(struct FS));
    child->path = (char*) kernel_alloc(strlen(path) + 1);
    strcpy(child->path, path);

    child->node = NULL;
    child->node_count = 0;
    child->parent = parent;
    child->next = NULL; // ���ļ���
    child->child = NULL;    // ͬ���ļ���

    // ���ӵ����ڵ��next��childͬ���б�
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
 * @brief ��ȡ�ӽڵ�
 * @param parent ���ڵ�
 * @param path �ӽڵ�·��
 * @return �ӽڵ�
 */
FS_t getFSChild(FS_t parent, char *path){
    FS_t p = parent->next;
    while(p != NULL){
        if(strcmp(p->path, path) == 0) return p;
        p = p->child;
    }
    return NULL;
}


// �����豸
/**
 * @brief �����豸
 * @param node �豸�ڵ�
 * @param devicePtr �豸ָ��(htim, huart, hspi)
 * @param name �豸����(��TIM1, USART1, SPI1)
 * @param description �豸����(�綨ʱ��1, ����1, SPI1)
 * @param type �豸����(��ͨ���豸��ʱ���豸)
 * @param driver �豸����(һ������ָ��)
 */
void addDevice(char *path, void* devicePtr, char *name, char *description, DeviceType_E type,
               DeviceStatus_E status, Func_t driver){
    FS_t node = getFSChild(RAM_FS, path);
    if (node == NULL) return;
    // �����豸�ڵ�
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

    // ���ӵ��豸����
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

/**
 * @brief ��ʼ���豸��
 */
void DrTInit(){
    // �������ڵ�RootFS�������б�
    RAM_FS = (FS_t) kernel_alloc(sizeof(struct FS));
    CMDList = (CMD_t) kernel_alloc(sizeof(struct CMD));
    CMDList->next = NULL;
    RAM_FS->path = "/";

    RAM_FS->node = NULL;
    RAM_FS->node_count = 0;

    RAM_FS->parent = NULL;
    RAM_FS->next = NULL;
    RAM_FS->child = NULL;

    // �����ն�·��
    currentFS = RAM_FS;

    // ������·��
    addFSChild(RAM_FS, "dev");
    addFSChild(RAM_FS, "tmp");
    addFSChild(RAM_FS, "mnt");
    addFSChild(RAM_FS, "bin");
    addFSChild(RAM_FS, "usr");
    addFSChild(RAM_FS, "root");
    addFSChild(RAM_FS, "opt");
    addFSChild(RAM_FS, "etc");
    addFSChild(RAM_FS, "proc");

    // �����豸
    addDevice("dev", &CortexM7, "Cortex-M7", "Central Processing Unit", DEVICE_BS, DEVICE_BUSY, NULL);
    addDevice("dev", &huart1, "USART1", "Serial bus device", DEVICE_SERIAL, DEVICE_BUSY, NULL);

    addDevice("mnt", &hsdram1, "SDMMC", "SD card", DEVICE_STORAGE, DEVICE_ON, NULL);
    addDevice("mnt", &SDFatFS, "FATFS", "FAT file system", FILE_SYSTEM, DEVICE_ON, NULL);
    addDevice("mnt", &hqspi, "QSPI", "Quad SPI", DEVICE_STORAGE, DEVICE_ON, NULL);


    // ָ��ע��
    register_main();
}


/**
 * @brief ����·��(��ȡ�ڵ�)
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

/**
 * @brief �����豸
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

        // token�ַ�����0˵��path���ж����·��
        if (strcmp(token, strtok(token, "/")) != 0){
            return NULL;
        } else {
            Task_t p = node->tasklist;
            while (p != NULL) {
                if (strcmp(p->name, token) == 0) return p;
                p = p->next;
            }
        }
    }

    return NULL;
}

// DrT����
// ====================================[ָ�����]===================================

/**
 * @brief ����Ŀ¼
 * @param path
 * @param name
 */
void ram_mkdir(char* path, char* name){
    FS_t node = loadPath(path);
    if(node == NULL) return;
    addFSChild(node, name);
}

/**
 * @brief �����ļ�
 * @param path
 * @param name
 */
void ram_mkfile(char* path, char* name){
    FS_t node = loadPath(path);
    if(node == NULL) return;
    addDevice(path, NULL, "file", "file", DrTFILE, DEVICE_ON, NULL);
}

/**
 * @brief ɾ��Ŀ¼
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
 * @brief ��ȡ�ļ�
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
 * @brief д���ļ�
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
 * @brief ��ʾĿ¼
 * @param path
 */
void ram_ls(char* path){
    FS_t node = loadPath(path);
    if(node == NULL) return;

    FS_t temp = node->next;

    while(temp != NULL){
        // ���node�����ļ���
        printf("%s  ", temp->path);
        temp = temp->child;
    }

    if (node->node_count != 0) {
        DrTNode_t p = node->node;
        while(p != NULL){
            printf("%s  ", p->name);
            p = p->next;
        }
    }

    if(node->tasklist != NULL){
        Task_t p = node->tasklist;
        while(p != NULL){
            printf("%s\t", p->name);
            p = p->next;
        }
    }

    printf("\n");
}

/**
 * @brief �л�Ŀ¼
 * @param path
 */
FS_t ram_cd(char* path){
    FS_t node = loadPath(path);
    if(node == NULL) return NULL;
    currentFS = node;
    return node;
}

/**
 * @brief ��ʾ��ǰĿ¼
 * @param fs ��ǰ�ļ���
 * @param path ·��(�Ѿ�ram_alloc)
 */
void ram_pwd(FS_t fs, char* path){
    FS_t temp_node;
    // �ݹ���ʾ·��
    if (fs == RAM_FS) {
        strcpy(path, "/");
        return;
    }else{
        temp_node = fs;
        ram_pwd(temp_node->parent, path);
        strcat(path, temp_node->path);
        strcat(path, "/");
    }

    path[strlen(path) - 1] = '\0';
}


// ============================[ָ�����]===========================

// ����ָ��
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

// ִ��ָ��
void execCMD(char* command_rel){
    // ����command, ��ֹcommand���޸�
    char* command = (char*) kernel_alloc(strlen(command_rel) + 1);
    strcpy(command, command_rel);

    // command����, command���ո�ָ�浽argv������
    char *argv[128] = {0};
    int argc = 0;
    char* token = strtok(command, " ");
    while(token != NULL){
        // ���ո�ָ�
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    argc -= 1;


    // ����ָ��
    CMD_t p = CMDList->next;
    while(p != NULL){
        if(strcmp(p->name, argv[0]) == 0){
            p->cmd(argc, &argv[1]);
            return;
        }
        p = p->next;
    }

    printf("Command not found\n");
}

void helpCMD(char *cmd){
    char buf[128];
    memoryCopy(buf, cmd, strlen(cmd) + 1);
    CMD_t p = CMDList->next;
    printf("Command\t\tDescription\t\tUsage\n");
    if(buf[0] == '\0') {
        while(p != NULL){
            printf("%s\t\t%s\t\t%s\n", p->name, p->description, p->usage);
            p = p->next;
        }
    }else{
        while(p != NULL){
            if(strcmp(p->name, cmd) == 0){
                printf("%s\t\t%s\t\t%s\n", p->name, p->description, p->usage);
                return;
            }
            p = p->next;
        }

    }
}