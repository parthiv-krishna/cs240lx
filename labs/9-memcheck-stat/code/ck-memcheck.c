// starter code for trivial heap checking using interrupts.
#include "rpi.h"
#include "rpi-internal.h"
#include "timer-interrupt.h"
#include "ckalloc.h"
#include "vector-base.h"
#include "breakpoint.h"


#define TIMER_INT_NCYCLES 0

// you'll need to pull your code from lab 2 here so you
// can fabricate jumps
// #include "armv6-insts.h"

// used to check initialization.
static volatile int init_p, check_on;

// allow them to limit checking to a range.  for simplicity we 
// only check a single contiguous range of code.  initialize to 
// the entire program.
static uint32_t 
    start_check = (uint32_t)&__code_start__, 
    end_check = (uint32_t)&__code_end__,
    // you will have to define these functions.
    start_nocheck = (uint32_t)ckalloc_start,
    end_nocheck = (uint32_t)ckalloc_end;

static int in_range(uint32_t addr, uint32_t b, uint32_t e) {
    assert(b<e);
    return addr >= b && addr < e;
}

// if <pc> is in the range we want to check and not in the 
// range we cannot check, return 1.
int (ck_mem_checked_pc)(uint32_t pc) {
    return in_range(pc, start_check, end_check) && 
          !in_range(pc, start_nocheck, end_nocheck);
}

// useful variables to track: how many times we did 
// checks, how many times we skipped them b/c <ck_mem_checked_pc>
// returned 0 (skipped)
static volatile unsigned checked = 0, skipped = 0;

unsigned ck_mem_stats(int clear_stats_p) { 
    unsigned s = skipped, c = checked, n = s+c;
    printk("total interrupts = %d, checked instructions = %d, skipped = %d\n",
        n,c,s);
    if(clear_stats_p)
        skipped = checked = 0;
    return c;
}

typedef struct {
    uint32_t offset:12, // bits 11:0
             Rd:4,      // bits 15:12
             Rn:4,      // bits 19:16
             L:1,       // bit 20
             W:1,       // bit 21
             B:1,       // bit 22
             U:1,       // bit 23
             P:1,       // bit 24
             I:1,       // bit 25
             single_transfer_flag:2, // bits 27:26 (0b01)
             cond:4;    // bits 31:28
} ldr_str_t;
_Static_assert(sizeof(ldr_str_t) == sizeof(uint32_t));

void validate_single_transfer(uint32_t *pc, uint32_t *regs) {
    ldr_str_t op = *(ldr_str_t *)pc;
    if (op.single_transfer_flag != 0b01) {
        // not a single transfer
        return;
    }

    if (op.Rn == 15) {
        // pc-relative addressing. we don't check this.
        return;
    }

    uint32_t *base = (uint32_t *) regs[op.Rn];

    // if bit set, then use register offset otherwise use immediate
    uint32_t offset = op.I ? regs[op.offset & 0b1111] : op.offset;

    // if bit set, add to base otherwise subtract
    uint32_t *target = op.U ? base + offset : base - offset;

    const char *op_name = op.L ? "load from" : "store to";

    if (!ck_ptr_is_alloced(target)) {
        panic("invalid %s %x. pc=%x, base=%x", op_name, target, pc, base);
    }
}

typedef struct {
    uint32_t reg_list:16, // bits 15:0
             Rn:4,        // bits 19:16
             L:1,         // bit 20
             W:1,         // bit 21
             S:1,         // bit 22
             U:1,         // bit 23
             P:1,         // bit 24
             multiple_transfer_flag:3, // bits 27:25 (0b100)
             cond:4;     // bits 31:28
} ldm_stm_t;

_Static_assert(sizeof(ldm_stm_t) == sizeof(uint32_t));

void validate_multiple_transfer(uint32_t *pc, uint32_t *regs) {
    ldm_stm_t op = *(ldm_stm_t *)pc;
    if (op.multiple_transfer_flag != 0b100) {
        // not a multiple transfer
        return;
    }

    if (op.Rn == 15) {
        // pc-relative or sp-relative addressing. we don't check this.
        return;
    }

    uint32_t *base = (uint32_t *) regs[op.Rn];
    unsigned n_regs = 0;

    // count regs
    uint32_t reg_list = op.reg_list;
    while (reg_list) {
        n_regs += reg_list & 1;
        reg_list >>= 1;
    }

    // if post-increment, need to consider one extra
    uint32_t offset = op.P ?  n_regs - 1: n_regs;
    // up or down?
    uint32_t *last = op.U ? base + offset : base - offset;

    const char *op_name = op.L ? "load from" : "store to";
    if (!ck_ptr_is_alloced(base)) {
        panic("invalid multiple %s base %x. pc=%x", op_name, base, pc);
    }
    if (!ck_ptr_is_alloced(last)) {
        panic("invalid multiple %s last %x. pc=%x, base=%x", op_name, last, pc, base);
    }

}

// note: lr = the pc that we were interrupted at.
// longer term: pass in the entire register bank so we can figure
// out more general questions.
void ck_mem_interrupt(uint32_t* pc, uint32_t *regs) {
    // we don't know what the user was doing.

    uint32_t instr = *pc;
    validate_single_transfer(pc, regs);
    validate_multiple_transfer(pc, regs);
}

void interrupt_vector(unsigned pc) {
    uint32_t *regs;
    asm volatile("mov %0, r1" : "=r"(regs));
    dev_barrier();
    // printk("abort\n");
    // system_disable_interrupts();

    // confirm we are in timer interrupt
    unsigned pending = GET32(IRQ_basic_pending);
    if((pending & RPI_BASIC_ARM_TIMER_IRQ) == 0) {
        dev_barrier();
        // system_enable_interrupts();
        return;
    }

    // printk("%p %x %d\n", pc, *(uint32_t *)pc, memory_op_type(*(uint32_t *)pc));

    if (check_on) {
        if (!ck_mem_checked_pc(pc)) {
            skipped++;
        } else {
            ck_mem_interrupt((uint32_t*)pc, regs);
            checked++;
        }
    }
    // printk("\n");

    // clear the interrupt
    PUT32(arm_timer_IRQClear, 1);
    // system_enable_interrupts();
    // we don't know what the user was doing.

    // brkpt_mismatch_set(pc);
    dev_barrier();
}


// do any interrupt init you need, etc.
void ck_mem_init(void) { 
    assert(!init_p);
    init_p = 1;

    assert(in_range((uint32_t)ckalloc, start_nocheck, end_nocheck));
    assert(in_range((uint32_t)ckfree, start_nocheck, end_nocheck));
    assert(!in_range((uint32_t)printk, start_nocheck, end_nocheck));

    skipped = 0;
    checked = 0;

    extern uint32_t _interrupt_table;
    vector_base_set(&_interrupt_table);
}

// only check pc addresses [start,end)
void ck_mem_set_range(void *start, void *end) {
    assert(start < end);
    start_check = (uint32_t)start;
    end_check = (uint32_t)end;
}

// maybe should always do the heap check at the begining
void ck_mem_on(void) {
    assert(init_p && !check_on);
    check_on = 1;

    timer_interrupt_init(TIMER_INT_NCYCLES);
    system_enable_interrupts();

    // uint32_t pc;
    // asm volatile("mov %0, r15" : "=r" (pc));
    // brkpt_mismatch_set(pc);
    // brkpt_mismatch_start();
    // asm volatile("cps 16");
    // system_enable_interrupts();
}

// maybe should always do the heap check at the end.
void ck_mem_off(void) {
    assert(init_p && check_on);
    check_on = 0;
    system_disable_interrupts();
    // brkpt_mismatch_stop();

}
