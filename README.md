# 使用stm32h743iit6实现的Linux系统

## 0. 项目预期
使用stm32h743iit6实现一个Linux系统，实现基本的Linux功能，如文件系统、网络功能、图形界面功能等。在系统中实现基本的指令集，包括对GPIO，对外设的指令操作，再通过对SD卡内指令程序文件的读取完成各种方便的功能。

> 前期时间紧张，等到时间宽裕些的时候写一个RTOS系统，再把Linux系统的功能移植到RTOS系统中。现在对付用FreeRTOS

初步计划指令集如下：
- 1. GPIO指令
1. gpio_read(port, pin) // 读取GPIO
2. gpio_write(port, pin, value) // 写入GPIO
3. gpio_toggle(port, pin) // 翻转GPIO

- 2. 外设指令
3. spi_read(name, data) // 读取SPI
4. spi_write(name, data) // 写入SPI
7. i2c_read(name, data) // 读取I2C
8. i2c_write(name, data) // 写入I2C
11. uart_read(name, data) // 读取UART
12. uart_write(name, data) // 写入UART
13. tim_setfreq(name, freq) // 设置定时器频率
14. tim_count(name) // 定时器计数
16. tim_pwm(name, duty) // 定时器PWM

- 3. SD卡指令
3. mkdir(path)
4. cd(path)
5. ls(path)
6. rm(path)
7. cp(src, dst)
8. mv(src, dst)
9. cat(path)
10. touch(path)
11. echo(str, path)
12. find(path)

## 1. 项目简介
本项目是基于stm32h743iit6实现的Linux系统，主要功能有：
- 1.1. 串口调试功能
- 1.2. 文件系统功能
- 1.3. 网络功能
- 1.4. 图形界面功能
- 1.5. 外设指令功能
- 1.6. SD卡指令开发功能
- 1.7. USB功能
- 1.8. 电源管理功能
- 1.9. 时钟管理功能
- 1.10. 线程调度算法

## 2. 项目结构
```
Appication // 应用程序
    |---inc // 应用程序
    `---src // 板级支持包
    
Command // 指令集
    |---cat // 读取文件
    |---cd // 切换目录
    |   |---cd_main.c 
    |   `---cd_main.h
    |---cp // 复制文件
    |---ls // 列出文件
    |   |---ls_main.c 
    |   `---ls_main.h
    |---mv // 移动文件
    |---tree // 显示目录树
    |---... // 其他指令
    |---Register.c // 指令注册
    `---Register.h // 指令注册头文件

Core // 核心文件
    |---inc // 头文件
    `---src // 源文件
    
Drivers // 驱动文件
    |---Kernel // 微缩std库
    |---CMSIS // Cortex-M7内核支持
    `---HAL // 硬件抽象层
    
Middlewares // 中间件
    |---FreeRTOS // 实时操作系统
    `---FatFS // USB OTG
    
FATFS // fat32文件系统
    |---App // 应用程序
    `---Target // 目标BSP
    
RAMFS // ram文件系统(未完成)
    `---init.d // 初始化脚本
    
USB_DEVICE // USB设备
    |---App // 应用程序
    `---Target // 目标BSP配置
    
Utilities // 工具, JPG解码
    `---JPEG // JPG解码
```

## 3. 项目进度
- [x] 3.1. 基本文件
  - [x] 3.1.1. memctl库
  - [x] 3.1.2. thread库
  - [x] 3.1.3. tlsf算法
  - [x] 3.1.4. timer库
- [x] 3.2. 文件系统功能
  - [x] 3.2.1. ram_alloc(size)函数
  - [x] 3.2.2. ram_free(ptr)函数
  - [x] 3.2.3. ram_realloc(ptr, size)函数
  - [x] 3.2.5. ram_create()初始化SDRAM为TLSF内存池
  - [x] 3.2.6. DrT设备树支持
  - [x] 3.2.7. SD挂载FATFS
  - [x] 3.2.8. RAMFS文件系统
  - [x] 3.2.9. 虚拟CPU
  - [x] 3.2.10. 文件系统指令
  - [x] 3.2.11. kernel_alloc(size)函数
  - [x] 3.2.12. kernel_free(ptr)函数
  - [x] 3.2.13. kernel_realloc(ptr, size)函数
  - [x] 3.2.14. kernel_create()初始化内核为TLSF内存池
- [x] 3.3. 外设指令功能
  - [x] 3.3.1. ls指令
  - [x] 3.3.2. cd指令
  - [x] 3.3.3. info指令
  - [x] 3.3.4. echo指令
  - [x] 3.3.5. help指令
  - [x] 3.3.6. tree指令
- [ ] 3.4. SD卡指令开发功能
- [x] 3.5. USB功能
  - [x] 3.5.1. USB printf功能
  - [x] 3.5.2. USB scanf功能
  - [x] 3.5.3. USB终端(自适应波特率)
- [ ] 3.6. 电源管理功能

## 4. 项目编译
下载程序后, 使用Clion打开工程，配置好工程后，点击编译即可。烧录软件为Openocd。环境使用了arm-none-eabi-gcc编译器。

## 5. 设备类型
FK743M2-IIT6

## 6. 项目作者
Minloha: https://blog.minloha.cn

## 7. 短期TODO
- [ ] 7.1. 完成SD卡指令开发功能
- [x] 7.2. 完成USB功能
- [ ] 7.3. 完成电源管理功能
- [x] 7.4. 完成时钟管理功能
- [x] 7.5. 完成外设指令功能
- [ ] 7.6. 完成图形界面功能
- [ ] 7.8. 完成网络功能
- [x] 7.9. 完成文件系统功能
- [x] 7.10. 完成串口调试功能
- [x] 7.11. 完成RAMFS文件系统
- [ ] 7.12. 完成BSP支持
- [ ] 7.13. 完成线程调度算法