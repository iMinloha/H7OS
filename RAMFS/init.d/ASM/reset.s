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
    ldr r0, =0x58024408
    ldr r1, [r0]
    mov r2, #0x04
    orr r1, r1, r2
    str r1, [r0]

    ldr r0, =0x58024408
    ldr r1, [r0]
    mov r2, #0x08
    orr r1, r1, r2
    str r1, [r0]

    // 进入DFU模式
    ldr r0, =0x05FA0004
    ldr r1, =0xE000ED0C
    str r0, [r1]
    dsb
    isb
    b .

