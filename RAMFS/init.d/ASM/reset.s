    .section .text
    .global system_reset
    .global shutdown_system
    .global jump_to_bootloader

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

/* Jump to STM32H7 system bootloader (DFU over USB OTG) */
jump_to_bootloader:
    ldr r0, =0x1FF09800
    ldr r1, [r0]
    msr msp, r1
    ldr r0, [r0, #4]
    bx r0
