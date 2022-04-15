gpio_lev0:
    .word 0x20200034

.globl loop_till_21_change
loop_till_21_change:
    ldr r2, =gpio_lev0
    ldr r2, [r2]
    ldr r1, [r2]
    tst r1, #(1<<21)
    beq loop_till_21_high_loop
    b   loop_till_21_low_loop

.align 4
loop_till_21_low_loop:
    ldr r1, [r2]
    mrc p15, 0, r0, c15, c12, 1 @ cycle_cnt_read()
    tst r1, #(1<<21)
    bne loop_till_21_low_loop
    bx lr


.align 4
loop_till_21_high_loop:
    ldr r1, [r2]
    mrc p15, 0, r0, c15, c12, 1 @ cycle_cnt_read()
    tst r1, #(1<<21)
    beq loop_till_21_high_loop
    bx lr
