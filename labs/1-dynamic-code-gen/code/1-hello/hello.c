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

    code[n++] = arm_sub_imm(arm_sp, arm_sp, 4); // sub sp, sp, #4
    code[n++] = arm_str_imm_off(arm_lr, arm_sp, 4); // str lr, [sp, #16]
    uint32_t bl = arm_bl((uint32_t) &code[n], (uint32_t) hello); 
    code[n++] = bl; // b hello
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

    // header stuff
    extern char __header_data__;
    printk("Message in header: `%s`\n", &__header_data__);
    const char *expected = "hello from header";

    assert(strcmp(&__header_data__, expected) == 0);

    printk("success!\n");

    clean_reboot();
}
