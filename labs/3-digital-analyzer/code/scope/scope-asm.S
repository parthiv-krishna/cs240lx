gpio_lev0:
    .word 0x20200034

.globl loop_till_21_low
loop_till_21_low:
    ldr r0, =gpio_lev0
    ldr r0, [r0]
    b   loop_till_21_low_loop

.align 4
loop_till_21_low_loop:
    ldr r1, [r0]
    tst r1, #(1<<21)
    bne loop_till_21_low_loop
    mrc p15, 0, r0, c15, c12, 1 @ cycle_cnt_read()
    bx lr

.globl loop_till_21_high
loop_till_21_high:
    ldr r0, =gpio_lev0
    ldr r0, [r0]
    b   loop_till_21_high_loop

.align 4
loop_till_21_high_loop:
    ldr r1, [r0]
    tst r1, #(1<<21)
    beq loop_till_21_high_loop
    mrc p15, 0, r0, c15, c12, 1 @ cycle_cnt_read()
    bx lr
