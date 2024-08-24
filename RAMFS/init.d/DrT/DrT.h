#ifndef H7OS_DRT_H
#define H7OS_DRT_H

// DrT: Device Tree
#include "u_thread.h"

/****************************************************
 * DrT 设备树
 *      - 设备树是一个树形结构，用于描述系统中的硬件设备。
 *      - 设备树是一个平台无关的描述硬件的数据结构。
 * */

enum DeviceType{
    DEVICE_TIMER,   // 定时器
    DEVICE_STORAGE, // 存储设备(qspi, sd, emmc)
    DEVICE_DISPLAY, // 显示设备(RGB, LVDS, HDMI)
    DEVICE_INPUT,   // 输入设备(OTG)
    DEVICE_SERIAL,  // 总线设备(USART, UART, SPI, IIC, CAN, LIN, USB)
    DEVICE_TRANSPORT,   // 传输设备(USB, ETH, WIFI)
    DEVICE_VOTAGE,  // 电压设备(ADC, DAC)
    // --------------------
    FILE_SYSTEM,    // 文件系统
    FILE,           // 文件
};

typedef enum DeviceType DeviceType_E;

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
    struct DrTNode* next;
    FS_t parent;
};

struct FS{
    // 路径
    char* path;

    // 子设备链表
    DrTNode_t node;
    // 设备数量
    int node_count;

    // 父级
    struct FS* parent;
    // 子级
    struct FS* next;
    // 层级
    struct FS* Level;
};

static FS_t RAM_FS;

// 初始化设备树(添加设备目录与分类)
void DrTInit();

DrTNode_t getDevice(char* path);

void getPath(char* path, FS_t node);

void displayDevice();

#endif
