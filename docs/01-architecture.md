# H7OS 系统架构 / Architecture

## 三层架构 / Three-Layer Architecture

```
┌──────────────────────────────────────────────────────────────┐
│ Software 层 / Software Layer                                 │
│ Applications + Command + RAMFS (DrT) + FATFS                 │
│ OS: device tree, RAM filesystem, shell, tasks                │
│ OS: 设备树, 内存文件系统, 命令行, 任务管理                       │
├──────────────────────────────────────────────────────────────┤
│ Platform 层 / Platform Layer                                 │
│ bsp_file_ops + board_select + bsp_init                       │
│ Unified interface, zero board dependency                     │
│ 统一接口, 零板级依赖                                           │
├──────────────────────────────────────────────────────────────┤
│ HAL 层 / HAL Layer                                           │
│ CubeMX generated code + board BSP drivers                    │
│ CubeMX 生成代码 + 板级 BSP 驱动                                 │
│ One folder per board / 每块板子一个独立目录                      │
└──────────────────────────────────────────────────────────────┘
```

### 调用规则 / Call Rules

```
Software → Platform → HAL        ✅ allowed / 允许
Software → HAL                   ❌ forbidden / 禁止
Platform → HAL                   ✅ allowed / 允许
HAL → Platform                   ❌ forbidden / 禁止
HAL → Software                   ❌ forbidden / 禁止
```

Software 层只能调用 Platform API (`dev_read`, `dev_write`, `ram_ls`, `ram_cd` 等)，不能直接调用 HAL 函数 (`HAL_GPIO_WritePin` 等)。

Software layer must only use Platform APIs, never HAL functions directly.

### 目录树 / Directory Tree

```
H7OS/
├── HAL/                          # HAL layer / HAL 层
│   ├── _template/                #   template for new boards / 新板子模板
│   └── FK743M2-IIT6/             #   FK743M2-IIT6 dev board / 开发板
│       ├── Inc/                  #     CubeMX headers / CubeMX 头文件
│       ├── Src/                  #     CubeMX sources / CubeMX 源文件
│       ├── Startup/              #     startup assembly / 启动汇编
│       ├── BSP/                  #     board BSP drivers / 板级 BSP 驱动
│       │   ├── hal_init.c        #       HAL_BoardInit() entry / 统一入口
│       │   ├── bsp_devices.c     #       devices_init + board_periph_init
│       │   ├── gpio/             #       GPIO driver / GPIO 驱动
│       │   ├── usart/            #       USART driver / 串口驱动
│       │   ├── pwm/              #       PWM driver / PWM 驱动
│       │   ├── adc/              #       ADC driver / ADC 驱动
│       │   ├── i2c/              #       I2C driver / I2C 驱动
│       │   ├── cpu/              #       CPU monitor / CPU 监控
│       │   ├── lcd/              #       LCD panel config / 液晶配置
│       │   └── touch/            #       touch screen / 触摸屏
│       ├── *.ioc                 #     CubeMX project / CubeMX 工程
│       └── *.ld                  #     linker script / 链接脚本
│
├── Platform/                     # Platform layer / Platform 层
│   ├── Board/                    #   board configs / 板级配置
│   │   ├── board_select.h        #     board selector / 板级选择器
│   │   ├── board_fk743m2.h       #     FK743M2 config / FK743M2 配置
│   │   └── board_template.h      #     template for new boards / 模板
│   ├── bsp_file_ops.h            #   file_ops interface / file_ops 接口
│   ├── bsp_devices.h             #   device registration API / 设备注册 API
│   ├── bsp_init.h                #   Platform_Init() declaration
│   └── bsp_init.c                #   Platform_Init() implementation
│
├── Core/                         # App entry / 应用入口
│   ├── Inc/FreeRTOSConfig.h      #   FreeRTOS config / FreeRTOS 配置
│   └── Src/                      #   main.c, freertos.c, syscalls, sysmem
│
├── Applications/                 # RTOS tasks / RTOS 任务
│   ├── inc/                      #   task headers / 任务头文件
│   └── src/                      #   task implementations / 任务实现
│       ├── xTaskInit.c           #     global init / 全局初始化
│       ├── xShellTask.c          #     USB CDC shell / USB CDC 终端
│       ├── xTaskManager.c        #     CPU load monitor / CPU 负载监控
│       └── xNoneTask.c           #     idle LED blink / 空闲 LED 闪烁
│
├── Command/                      # Shell commands / Shell 命令
│   ├── Register.h/.c             #   command registry / 命令注册表
│   ├── ls/ cd/ pwd/ cat/ echo/   #   filesystem commands / 文件系统命令
│   ├── mkdir/ touch/ rm/ cp/ mv/ #   file ops / 文件操作
│   ├── mount/ tree/ use/ info/   #   system commands / 系统命令
│   ├── help/ save/ reboot/       #   utility / 工具
│   └── reset/ flash/ dfu/        #   maintenance / 维护
│
├── RAMFS/                        # DrT device tree + RAM filesystem
│   └── init.d/
│       ├── Core/DrT.h, DrT.c     #   core data structures / 核心数据结构
│       ├── FS/DrT_fs.c           #   filesystem ops (ls,cd,touch,rm,mkdir)
│       ├── Dev/DrT_dev.c         #   device registration + I/O / 设备注册
│       ├── Cmd/DrT_cmd.c         #   command registry + exec / 命令注册
│       ├── Mnt/DrT_mnt.c         #   SD mount path translation / SD 路径
│       └── Proc/DrT_proc.c       #   task management / 任务管理 (/proc)
│
├── FATFS/                        # FatFs integration / FatFs 集成
│   ├── App/                      #   application layer / 应用层
│   └── Target/                   #   SD disk I/O + ffconf.h
│
├── USB_DEVICE/                   # USB CDC virtual COM / USB CDC 虚拟串口
├── Drivers/                      # STM32 HAL + CMSIS + Kernel / 驱动
├── Middlewares/                  # FreeRTOS + FatFs + USB / 中间件
├── docs/                         # documentation / 文档
└── CMakeLists.txt                # build system / 构建系统
```

### 板级切换 / Board Switching

```
cmake -DBOARD=Fk743M2-IIT6 →  HAL/FK743M2-IIT6/ 编译
cmake -DBOARD=MyBoard     →  HAL/MyBoard/     编译
```

CMake 自动:
- 添加 `HAL/${BOARD}/Inc` 和 `HAL/${BOARD}/BSP` 到头文件路径
- 编译 `HAL/${BOARD}/` 下所有源码
- 排除 CubeMX 生成的 `HAL/${BOARD}/Src/main.c`
- 匹配 `HAL/${BOARD}/*_FLASH.ld` 作为链接脚本
- 生成 `-DBOARD_XXX` 宏激活对应板级配置
