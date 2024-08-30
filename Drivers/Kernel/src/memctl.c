#include "memctl.h"
#include "quadspi.h"
#include "tlsf.h"

/***
 * @brief 内存拷贝
 * @param dest: destination address
 * @param src: source address
 * @param size: size of the memory block
 * */
void memoryCopy(void *dest, void *src, int size){
    char *d = (char *)dest;
    char *s = (char *)src;
    for(int i = 0; i < size; i++) d[i] = s[i];
}

/***
 * @brief 内存设置
 * @param dest: destination address
 * @param val: value to set
 * @param size: size of the memory block
 * */
void memorySet(void *dest, char val, int size){
    char *d = (char *)dest;
    for(int i = 0; i < size; i++) d[i] = val;
}

/***
 * @brief 内存比较
 * @param dest: destination address
 * @param src: source address
 * @param size: size of the memory block
 * @return 比较结果
 * */
int memoryCompare(void *dest, void *src, int size){
    char *d = (char *)dest;
    char *s = (char *)src;
    for(int i = 0; i < size; i++) if(d[i] != s[i]) return 0;
    return 1;
}

/***
 * @brief QSPI flash写入数据
 * @param addr: 写入地址(从0x0000开始到0x0400)
 * @param data: 写入数据
 * @param size: 写入数据大小
 * */
void flashWrite(uint32_t addr, uint8_t *data, uint32_t size){
    QSPI_W25Qxx_WriteBuffer(data, addr, size);
}

/***
 * @brief QSPI flash读取数据
 * @param addr: 读取地址
 * @param data: 读取数据存放地址
 * @param size: 读取数据大小
 * */
void flashRead(uint32_t addr, uint8_t *data, uint32_t size){
    QSPI_W25Qxx_ReadBuffer(data, addr, size);
}

// ========================= 以下是SDRAM内存管理算法 =========================
/***
 * @brief 刷新SDRAM
 * */
void flashSDRAM(){
    for (uint32_t i = 0; i < SDRAM_Size; i++) IO_U8 (SDRAM_BANK_ADDR+i) = 0;
}

// 内存池
tlsf_t mem_pool;
tlsf_t kernel_pool;
// 使用内存
uint32_t using_mem = 0;

/***
 * @brief SDRAM内存管理初始化
 * @note 该函数会初始化SDRAM内存管理算法, 请在使用SDRAM内存管理前调用该函数.
 * */
void MemControl_Init(){
    flashSDRAM();
    // 内核空间(会自动保存在QSPI Flash中)
    kernel_pool = tlsf_create_with_pool((void*)SDRAM_BANK_ADDR, 2 * 1024 * 1024);
    // 程序运行空间
    mem_pool = tlsf_create_with_pool((void*)SDRAM_BANK_ADDR + 2 * 1024 * 1024, 30 * 1024 * 1024);
}

/***
 * @brief SDRAM内存管理分配
 * @param size: 分配内存大小
 * @return 分配内存的地址
 * */
void* ram_alloc(uint32_t size){
    using_mem += size;
    return tlsf_malloc(mem_pool, size);
}

/***
 * @brief SDRAM内存管理重新分配
 * @param addr: 重新分配内存的地址
 * @param size: 重新分配内存大小
 * @return 重新分配内存的地址
 * */
void* ram_realloc(void* addr, uint32_t size){
    return tlsf_realloc(mem_pool, addr, size);
}

/***
 * @brief SDRAM内存管理释放
 * @param addr: 释放内存的地址
 * */
void ram_free(void* addr){
    using_mem -= tlsf_block_size(addr);
    tlsf_free(mem_pool, addr);
}

/***
 * @brief SDRAM内存管理检查
 * @return 检查结果
 * */
int ram_check(){
    return tlsf_check(mem_pool);
}

/***
 * @brief SDRAM内存管理信息
 * */
void ram_info(){
    if (using_mem < 1024) u_print("Using memory: %d bytes\n", using_mem);
    else if (using_mem < 1024*1024) u_print("Using memory: %d KB\n", using_mem/1024);
    else u_print("Using memory: %d MB\n", using_mem/1024/1024);
}

/***
 * @brief SDRAM内存管理销毁
 * @note 该函数会销毁SDRAM内存管理算法, 请在不使用SDRAM内存管理后调用该函数.
 * */
void ram_destroy(){
    tlsf_destroy(mem_pool);
}

/***
 * @brief SDRAM内存管理对齐
 * @param align: 对齐大小
 * @param bytes: 对齐字节数
 * */
void ram_align(size_t align, size_t bytes){
    tlsf_memalign(mem_pool, align, bytes);
}

void* kernel_alloc(uint32_t size){
    return tlsf_malloc(kernel_pool, size);
}

void* kernel_realloc(void* addr, uint32_t size){
    return tlsf_realloc(kernel_pool, addr, size);
}

void kernel_free(void* addr){
    tlsf_free(kernel_pool, addr);
}
