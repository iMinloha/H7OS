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
    DEVICE_SERIAL,  // 串口设备
    DEVICE_TRANSPORT,   // 传输设备(USB, ETH, WIFI)
    DEVICE_VOTAGE,  // 电压设备(ADC, DAC)
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
    // 设备驱动
    Func_t driver;
    // --------------------
    Mutex_t mutex;  // 设备锁
    // --------------------
    struct DrTNode* next;
};

typedef struct DrTNode* DrTNode_t;

#endif
