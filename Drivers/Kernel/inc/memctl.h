#ifndef H7OS_MEMCTL_H
#define H7OS_MEMCTL_H

#include "fmc.h"

#define LOGGER printf

// 定义内存管理算法
/***
 * @brief 内存拷贝
 * @param dest: destination address
 * @param src: source address
 * @param size: size of the memory block
 * */
void memoryCopy(void *dest, void *src, int size);

/***
 * @brief 内存设置
 * @param dest: destination address
 * @param val: value to set
 * @param size: size of the memory block
 * */
void memorySet(void *dest, char val, int size);

/***
 * @brief 内存比较
 * @param dest: destination address
 * @param src: source address
 * @param size: size of the memory block
 * @return 比较结果
 * */
int memoryCompare(void *dest, void *src, int size);


// ========================= 以下是QSPI Flash内存管理算法 =========================
/***
 * @brief QSPI flash写入数据
 * @param addr: 写入地址(从0x0000开始到0x0400)
 * @param data: 写入数据
 * @param size: 写入数据大小
 * */
void flashWrite(uint32_t addr, uint8_t *data, uint32_t size);

/***
 * @brief QSPI flash读取数据
 * @param addr: 读取地址
 * @param data: 读取数据存放地址
 * @param size: 读取数据大小
 * */
void flashRead(uint32_t addr, uint8_t *data, uint32_t size);

// ========================= 以下是SDRAM内存管理算法 =========================

// 定义IO操作
#define IO_U8 *(__IO uint8_t*)  // 定义IO_U8为一个指向uint8_t的指针
#define IO_U16 *(__IO uint16_t*)    // 定义IO_U16为一个指向uint16_t的指针
#define IO_U32 *(__IO uint32_t*)    // 定义IO_U32为一个指向uint32_t的指针
#define IO_U64 *(__IO uint64_t*)    // 定义IO_U64为一个指向uint64_t的指针
#define IO_VOID *(__IO void*)    // 定义IO_VOID为一个指向void的指针
#define IO_STRUCT(Pointer) *(__IO Pointer)  // 定义IO_STRUCT为一个指向结构体或函数的指针
// 定义IO操作结束
#define VideoMemSize 8 * 1024 * 1024  // 显存大小8MB
#define KernelMemSize 12 * 1024 * 1024  // 内核内存12MB
#define UserMemSize 12 * 1024 * 1024   // SDRAM内存12MB

/***
 * @brief SDRAM内存管理初始化
 * @note 该函数会初始化SDRAM内存管理算法, 请在使用SDRAM内存管理前调用该函数.
 * */
void MemControl_Init();

/***
 * @brief SDRAM内存管理分配
 * @param size: 分配内存大小
 * @return 分配内存的地址
 * */
void* ram_alloc(uint32_t size);


/***
 * @brief 内核内存管理分配
 * @param size: 分配内存大小
 * @return 分配内存的地址
 * */
void* kernel_alloc(uint32_t size);


/***
 * @brief SDRAM内存管理重新分配
 * @param addr: 重新分配内存的地址
 * @param size: 重新分配内存大小
 * @return 重新分配内存的地址
 * */
void* ram_realloc(void* addr, uint32_t size);

/***
 * @brief 内核内存管理重新分配
 * @param addr: 重新分配内存的地址
 * @param size: 重新分配内存大小
 * @return 重新分配内存的地址
 * */
void* kernel_realloc(void* addr, uint32_t size);

/***
 * @brief SDRAM内存管理释放
 * @param addr: 释放内存的地址
 * */
void ram_free(void* addr);

/***
 * @brief ram内存查看
 * */
int ram_check();

/***
 * @brief 内核内存管理释放
 * @param addr: 释放内存的地址
 * */
void kernel_free(void* addr);

/***
 * @brief SDRAM内存管理信息
 * */
void ram_info();

/***
 * @brief SDRAM内存管理销毁
 * @note 该函数会销毁SDRAM内存管理算法, 请在不使用SDRAM内存管理后调用该函数.
 * */
void ram_destroy();

/***
 * @brief SDRAM内存管理对齐
 * @param align: 对齐大小
 * @param bytes: 对齐字节数
 * */
void ram_align(size_t align, size_t bytes);


#endif
