# 文件系统 / Filesystem

## DrT — 设备树与 RAM 文件系统 / Device Tree & RAM Filesystem

DrT (Device Tree) 是 H7OS 的核心数据结构，运行在 SDRAM 的 TLSF 内存池上。

### 数据结构 / Data Structures

**FS (目录节点 / Directory Node)**:
```c
struct FS {
    char      *path;          // 目录名 / dir name
    DrTNode_t  node;          // 第一个子设备/文件 / first child device/file
    int        node_count;    // 子节点数 / child count
    Task_t     tasklist;      // 任务列表 / task list (for /proc)
    uint8_t    allow_rm;      // 可删除标志 / removable flag
    char      *sd_mount_path; // SD 挂载基础路径 / SD mount base path
    char      *sd_cd_path;    // SD 当前子路径 / SD current sub-path
    FS_t       parent;        // 父目录 / parent
    FS_t       child_next;    // 第一个子目录 / first child
    FS_t       level_next;    // 下一个兄弟 / next sibling
};
```

**DrTNode (设备/文件节点 / Device/File Node)**:
```c
struct DrTNode {
    void          *device;    // HAL 句柄 / HAL handle (&huart1, &hspi1)
    DeviceStatus_E status;    // 设备状态 / device status
    DeviceType_E   type;      // 设备类型 / device type
    char          *name;      // 名称 / name
    char          *description; // 描述 / description
    void          *data;      // 文件数据 / file data buffer
    void          *fops;      // bsp_file_ops_t 函数表 / file operations
    osThreadId     owner;     // 打开该设备的任务 / task that opened this device
    Mutex_t        mutex;     // 互斥锁 / mutex
    DrTNode_t      next;      // 下一个兄弟 / next sibling
    FS_t           parent;    // 父目录 / parent directory
};
```

### 目录树 / Directory Tree

```
/ (root)
├── dev/                      # 设备节点 / device nodes
│   ├── cpu/CPU               #   CPU 监控 / CPU monitor
│   ├── serial/USART1         #   USART1 串口
│   ├── gpio/PF7, gpio/PH7    #   GPIO 引脚
│   ├── pwm/PWM2_CH1, PWM2_CH2 # PWM 通道
│   ├── adc/ADC1              #   ADC 输入
│   └── i2c/I2C1              #   I2C 总线
├── mnt/                      # 挂载点 / mount points
│   ├── QSPI                  #   QSPI Flash 存储
│   └── SDcard                #   SD 卡 / SD card (FAT32/exFAT)
├── usr/                      # 用户数据 / user data (持久化到 QSPI)
├── proc/                     # 进程 / processes
│   ├── Shell                 #   Shell 任务
│   ├── TaskMgr               #   任务管理器 / task manager
│   ├── Test                  #   测试任务
│   └── Kernel                #   空闲任务 / idle task
└── bin/                      # 保留 / reserved for apps
```

### API

| 函数 / Function | 说明 / Description |
|---|---|
| `DrTInit()` | 初始化 DrT 树 + 创建 `/dev`, `/mnt`, `/usr`, `/proc`, `/bin` |
| `loadPath(path)` | 解析路径到 FS 节点 / resolve path to FS node |
| `loadDevice(path)` | 解析路径到 DrTNode / resolve path to DrTNode |
| `addDevice(dir, dev, name, desc, type, status)` | 注册设备到 DrT / register device |
| `ram_ls(path)` | 列出目录内容 / list directory |
| `ram_cd(path)` | 切换当前目录 / change directory |
| `ram_pwd(fs, buf)` | 获取当前路径 / get current path |
| `ram_touch(path, data, len)` | 创建文件 / create file |
| `ram_rm(path)` | 删除文件或目录 / remove file or directory |
| `ram_deep_mkdir(path)` | 递归创建目录 / recursive mkdir |
| `fs_to_sd_path(rel)` | 相对路径 → FatFs 完整路径 (含 "0:" 前缀) |
| `dev_open(path)` | 打开设备 (状态 → BUSY) / open device |
| `dev_close(path)` | 关闭设备 (状态 → ON) / close device |
| `dev_read(path, buf, len)` | 读设备 / read device |
| `dev_write(path, buf, len)` | 写设备 / write device |

### SD 卡路径转换 / SD Path Translation

SD 卡通过 FatFs 驱动访问，路径格式为 `"0:/dir/file"`。
当用户在挂载点内操作时，`fs_to_sd_path()` 自动拼接驱动器前缀。

```
用户输入 / user types:    cd /mnt/SDcard; ls
内部分辨 / internal:      fs_to_sd_path("") → "0:" → f_opendir("0:")
用户输入:                 touch example
内部分辨:                 fs_to_sd_path("example") → "0:/example" → f_open
```
