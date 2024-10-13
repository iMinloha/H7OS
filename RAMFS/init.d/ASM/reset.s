    .section .text
    .global system_reset    // 系统复位
    .global shutdown_system // 关机(低功耗模式，除非触发中断)
    .global dfu_mode  //

system_reset:
    dsb
    ldr r0, =0x05FA0004
    ldr r1, =0xE000ED0C
    str r0, [r1]
    dsb
    isb
    b .

shutdown_system:
    cpsid i
    ldr r0, =0xE000ED10
    ldr r1, [r0]
    mov r2, #0x04
    orr r1, r1, r2
    str r1, [r0]

    ldr r0, =0x58024408
    ldr r1, [r0]
    orr r1, r1, r2
    str r1, [r0]
    wfi

dfu_mode:
    ldr r0, =0x1FF00000         // 系统引导程序的地址
    ldr r1, =0x20020000         // 初始堆栈指针的地址
    ldr r2, [r1]                // 将堆栈指针值加载到 r2
    mov sp, r2                  // 将 r2 的值赋给堆栈指针 sp
    ldr r1, [r0]                // 读取引导程序的复位向量地址
    bx r1                       // 跳转到引导程序
