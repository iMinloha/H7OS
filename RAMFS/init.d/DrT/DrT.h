#ifndef H7OS_DRT_H
#define H7OS_DRT_H

// DrT: Device Tree
#include "u_thread.h"
#include "Register.h"
#include "xTaskManager.h"

/****************************************************
 * DrT 设备树
 *      - 设备树是一个树形结构，用于描述系统中的硬件设备。
 *      - 设备树是一个平台无关的描述硬件的数据结构。
 *      - 文件包含：DrT的创建和声明以及指令处理
 * */

enum DeviceType{
    DEVICE_TIMER,   // 定时器
    DEVICE_BS,      // 基设备
    DEVICE_STORAGE, // 存储设备(qspi, sd, emmc)
    DEVICE_DISPLAY, // 显示设备(RGB, LVDS, HDMI)
    DEVICE_INPUT,   // 输入设备(OTG)
    DEVICE_SERIAL,  // 总线设备(USART, UART, SPI, IIC, CAN, LIN, USB)
    DEVICE_TRANSPORT,   // 传输设备(USB, ETH, WIFI)
    DEVICE_VOTAGE,  // 电压设备(ADC, DAC)
    DEVICE_TASK,    // 任务设备(proc显示的内容)
    // --------------------
    FILE_SYSTEM,    // 文件系统
    DrTFILE,           // 文件
};

typedef enum DeviceType DeviceType_E;

// 设备状态
enum DeviceStatus{
    DEVICE_OFF,     // 关闭
    DEVICE_ON,      // 打开
    DEVICE_SUSPEND, // 挂起
    DEVICE_ERROR,   // 错误
    DEVICE_BUSY,    // 占用
};

typedef enum DeviceStatus DeviceStatus_E;
typedef struct FS* FS_t;
typedef struct DrTNode* DrTNode_t;

struct DrTNode{
    // 设备地址
    void* device;
    // 设备状态
    DeviceStatus_E status;
    // 设备类型
    DeviceType_E type;
    // 设备名称
    char* name;
    // 设备描述
    char* description;
    // 设备数据缓冲
    void* data;
    // --------------------
    // 设备驱动
    Func_t driver;
    // --------------------
    Mutex_t mutex;  // 设备锁
    // --------------------
    DrTNode_t next;
    FS_t parent;
};

struct FS{
    // 路径
    char* path;

    // 子设备链表
    DrTNode_t node;
    // 设备数量
    int node_count;
    // --------------------
    // proc目录子项目
    Task_t tasklist;

    // 父级
    FS_t parent;
    // 层级
    FS_t next;
    // 子级
    FS_t child;
};

// 根文件系统
static FS_t RAM_FS;

// 初始化设备树(添加设备目录与分类)
void DrTInit();

// 判断kernel的ramfs是否存在
void checkDrT();
// 保存设备树
void saveDrT();


// ===============================[指令操作]===============================

typedef void (*Comand_t)(int argc, char **argv);

typedef struct CMD* CMD_t;

struct CMD{
    char* name; // 指令名，如mkdir, rm, ls
    char* description;  // 指令描述, 用于help
    char* usage;    // 指令使用方法
    /***
     * @brief 指令函数要求
     * @param argc 参数数量
     * @param argv 参数列表(char**)
     * */
    Comand_t cmd; // 指令主函数
    CMD_t next; // 下一个指令
};

static CMD_t CMDList;

// 添加指令
void addCMD(char* name, char* description, char* usage, Comand_t cmd);

// 执行指令
void execCMD(char* command);

void helpCMD(char *cmd);

#define CMD(name, description, usage, cmd) addCMD(name, description, usage, cmd)

// ===============================[设备操作]===============================

/**
 * @brief 创建目录
 * @param path
 * @param name
 */
void ram_mkdir(char* path, char* name);

/**
 * @brief 创建文件
 * @param path
 * @param name
 */
void ram_mkfile(char* path, char* name);

/**
 * @brief 删除目录
 * @param path
 * @param name
 */
void ram_rm(char* path, char *name);

/**
 * @brief 读取文件
 * @param path
 * @param buf
 * @param size
 */
void ram_read(char* path, void* buf, int size);

/**
 * @brief 写入文件
 * @param path
 * @param buf
 * @param size
 */
void ram_write(char* path, void* buf, int size);

/**
 * @brief 显示目录
 * @param path
 */
void ram_ls(char* path);

/**
 * @brief 切换目录
 * @param path
 */
FS_t ram_cd(char* path);

/**
 * @brief 显示当前目录
 * @param fs 当前文件夹
 * @param path 路径(已经ram_alloc)
 */
void ram_pwd(FS_t fs, char* path);

/**
 * @brief 加载路径,返回目录
 * @param path
 * @return
 */
FS_t loadPath(char* path);

/**
 * @brief 加载设备
 * @param path
 * @return
 */
DrTNode_t loadDevice(char* path);

// ===============================[任务操作]===============================

/**
 * @brief 加载任务信息
 * @param path
 * @return
 */
Task_t loadTask(char* path);

/**
 * @brief 添加设备
 * @param task 任务结构体
 * */
void addThread(Task_t task);

/**
 * @brief 使用线程名获取线程结构体
 * @param name 线程名
 * */
Task_t getThread(char* name);

/**
 * @brief 使用线程PID获取线程结构体
 * @param pid 线程id
 * */
Task_t getThreadByPID(uint8_t pid);

/**
 * @brief 获取任务列表
 * */
Task_t getTaskList();

/**
 * @brief 获取任务
 * @param handle 任务句柄
 * */
Task_t getTaskByHandle(osThreadId handle);

#endif
