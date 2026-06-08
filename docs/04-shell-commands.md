# Shell 命令参考 / Shell Command Reference

Shell 通过 USB CDC 虚拟串口 (USART2) 交互。提示符: `root:/path$`

## 命令列表 / Command List

### 文件系统 / Filesystem

| 命令 | 用法 | 说明 |
|------|------|------|
| `ls [path]` | `ls`, `ls /dev` | 列出目录 / list directory |
| `cd <path>` | `cd /mnt/SDcard` | 切换目录 / change directory |
| `pwd` | `pwd` | 显示当前路径 / print working directory |
| `cat <file>` | `cat readme.txt` | 查看文件内容 / print file (RAMFS or SD) |
| `echo <text...>` | `echo hello world` | 输出文本 / print text |
| `touch <path> [content]` | `touch /usr/config.ini` | 创建文件 / create file |
| `mkdir <path>` | `mkdir /usr/data` | 创建目录 / create directory (RAMFS or SD) |
| `rm <path>` | `rm /usr/old.txt` | 删除文件或目录 / remove file or directory |
| `cp <src> <dst>` | `cp /usr/a.txt /mnt/SDcard/b.txt` | 复制文件 / copy file |
| `mv <src> <dst>` | `mv a.txt b.txt` | 移动/重命名 / move/rename (SD→SD 为原地 rename) |
| `tree [depth]` | `tree 2` | 树形显示目录 / show directory tree |

### 挂载 / Mount

| 命令 | 用法 | 说明 |
|------|------|------|
| `mount <ramfs_path>` | `mount /sd` | 挂载 SD 根目录到 RAMFS / mount SD root |
| `mount <sd_path> <ramfs_path>` | `mount /photos /mnt/pics` | 挂载 SD 子目录 |

挂载后 `cd` 进入该目录即可操作 SD 卡文件。

### 设备操作 / Device I/O

| 命令 | 用法 | 说明 |
|------|------|------|
| `use <device> open` | `use /dev/gpio/PH7 open` | 打开设备 / open device |
| `use <device> close` | `use /dev/gpio/PH7 close` | 关闭设备 / close device |
| `use <device> read [n]` | `use /dev/adc/ADC1 read` | 读取设备 / read device value |
| `use <device> write <data>` | `use /dev/gpio/PH7 write 1` | 写入设备 / write to device |

### 信息 / Information

| 命令 | 用法 | 说明 |
|------|------|------|
| `info <device>` | `info /dev/cpu/CPU` | 显示设备详情 / show device info |
| `info <task>` | `info Shell` | 显示任务详情 / show task info |
| `help [cmd]` | `help`, `help ls` | 显示帮助 / show help |

### 系统 / System

| 命令 | 用法 | 说明 |
|------|------|------|
| `save` | `save` | 保存 RAMFS 到 QSPI Flash / save filesystem |
| `reboot` | `reboot` | 保存并重启 / save and reboot |
| `reset` | `reset` | 擦除 QSPI Flash 并复位 / erase flash and reset |
| `flash` | `flash` | 擦除 QSPI Flash (保留当前系统) / erase only |
| `dfu` | `dfu` | 进入 DFU 固件升级模式 / enter DFU mode |

### 设备路径 / Device Paths

| 路径 | 操作 |
|------|------|
| `/dev/gpio/PH7` | LED: 读返回 0/1, 写 1 亮 0 灭 / read 0/1, write 1=on 0=off |
| `/dev/gpio/PF7` | 同上 |
| `/dev/adc/ADC1` | ADC1: 读返回电压值 / read returns raw ADC value |
| `/dev/pwm/PWM2_CH1` | PWM: 写 0-100 设置占空比% / write 0-100 for duty cycle |
| `/dev/pwm/PWM2_CH2` | 同上 |
| `/dev/i2c/I2C1` | I2C: 读/写, 首字节为从机地址 / first byte = slave addr |
| `/dev/serial/USART1` | USART1: 非阻塞读, 写发送 / non-blocking read, write sends |
| `/dev/cpu/CPU` | CPU 信息: 频率, 温度, 负载 / freq, temp, load |

### 文件读写限制 / File Size Limits

- **RAMFS**: 单个文件最大 127 字节 / max 127 bytes per file
- **SD 卡**: 无限制 (FatFs) / unlimited
- 从 SD→RAMFS 复制超过 127 字节的文件会被拒绝 / files >127 bytes rejected
