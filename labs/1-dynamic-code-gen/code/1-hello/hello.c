#include "rpi.h"
#include "../unix-side/armv6-insts.h"

void hello(void) { 
    printk("hello world\n");
}

// i would call this instead of printk if you have problems getting
// ldr figured out.
void foo(int x) { 
    printk("foo was passed %d\n", x);
}

void notmain(void) {
    // generate a dynamic call to hello world.
    // 1. you'll have to save/restore registers
    // 2. load the string address [likely using ldr]
    // 3. call printk
    static uint32_t code[16];
    unsigned n = 0;

    // asm volatile("mov r1, %0" : : "r" (printk_addr) : "r1");

    // code[n++] = arm_sub_imm(arm_sp, arm_sp, 4); // sub sp, sp, #4
    // code[n++] = arm_str_no_off(arm_r0, arm_sp); // str r0, [sp, #0]
    // code[n++] = arm_blx(arm_r0);                    // printk(hello_str)
    // code[n++] = arm_ldr_no_off(arm_r0, arm_sp); // ldr r0, [sp, #0]
    // code[n++] = arm_add_imm(arm_sp, arm_sp, 4); // add sp, sp, #4

    code[n++] = arm_sub_imm(arm_sp, arm_sp, 4); // sub sp, sp, #4
    // code[n++] = arm_str_imm_off(arm_r0, arm_sp, 0); // str r0, [sp, #0]
    // code[n++] = arm_str_imm_off(arm_r1, arm_sp, 4); // str r1, [sp, #4]
    // code[n++] = arm_str_imm_off(arm_r2, arm_sp, 8); // str r2, [sp, #8]
    // code[n++] = arm_str_imm_off(arm_r3, arm_sp, 12); // str r3, [sp, #12]
    code[n++] = arm_str_imm_off(arm_lr, arm_sp, 4); // str lr, [sp, #16]

    int32_t offset = (int32_t)&hello - ((int32_t)&code[n] + 8);
    printk("offset: %d code: %p\n", offset, code);
    code[n++] = arm_bl(offset); // b hello

    // code[n++] = arm_ldr_imm_off(arm_r0, arm_sp, 0); // ldr r0, [sp, #0]
    // code[n++] = arm_ldr_imm_off(arm_r1, arm_sp, 4); // ldr r1, [sp, #4]
    // code[n++] = arm_ldr_imm_off(arm_r2, arm_sp, 8); // ldr r2, [sp, #8]
    // code[n++] = arm_ldr_imm_off(arm_r3, arm_sp, 12); // ldr r3, [sp, #12]
    code[n++] = arm_ldr_imm_off(arm_lr, arm_sp, 4); // ldr lr, [sp, #16]
    code[n++] = arm_add_imm(arm_sp, arm_sp, 4); // add sp, sp, #16

    code[n++] = arm_bx(arm_lr); // bx lr

    printk("emitted code:\n");
    for(int i = 0; i < n; i++) 
        printk("code[%d]=0x%x\n", i, code[i]);

    void (*fp)(void) = (typeof(fp))code;
    printk("about to call: %x\n", fp);
    printk("--------------------------------------\n");
    fp();
    printk("--------------------------------------\n");
    printk("success!\n");
    clean_reboot();
}
