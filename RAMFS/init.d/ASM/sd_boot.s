    .section .sd_boot, "ax"
    .global sd_boot
    .extern f_open
    .extern f_read
    .extern f_close
    .extern memcpy
    .extern printf

sd_boot:
    // 打开boot.bin文件
    ldr r0, =boot_bin
    ldr r1, =0x0
    ldr r2, =0x0
    bl f_open
    cmp r0, #0
    bne .L1
    b .L2
.L1:
    // 读取boot.bin文件
    ldr r0, =boot_bin
    ldr r1, =0x0
    ldr r2, =0x1000
    bl f_read
    cmp r0, #0
    bne .L3
    b .L2
.L3:
    // 关闭boot.bin文件
    ldr r0, =boot_bin
    bl f_close
    // 复制boot.bin文件到0x10000000
    ldr r0, =0x10000000
    ldr r1, =boot_bin
    ldr r2, =0x1000
    bl memcpy
    ldr r0, =0x10000000
    bx r0
.L2:
    ldr r0, =.L2_str
    bl printf
    b .
.L2_str:
    .asciz "open boot.bin failed\n"
    .size sd_boot, .-sd_boot