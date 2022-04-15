gpio_lev0:
    .word 0x20200034

.globl loop_till_21_low
loop_till_21_low:
    ldr r2, =gpio_lev0
    ldr r2, [r2]
    b   loop_till_21_low_loop

.align 4
loop_till_21_low_loop:
    mrc p15, 0, r0, c15, c12, 1 @ cycle_cnt_read()
    ldr r1, [r2]
    tst r1, #(1<<21)
    bne loop_till_21_low_loop
    bx lr





.globl loop_till_21_high
loop_till_21_high:
    ldr r2, =gpio_lev0
    ldr r2, [r2]
    b   loop_till_21_high_loop

.align 4
loop_till_21_high_loop:
    mrc p15, 0, r0, c15, c12, 1 @ cycle_cnt_read()
    ldr r1, [r2]
    tst r1, #(1<<21)
    beq loop_till_21_high_loop
    bx lr
