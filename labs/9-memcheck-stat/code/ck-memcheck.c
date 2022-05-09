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

typedef enum {
    MEM_OP_NONE = 0,
    MEM_OP_SINGLE = 1,
    MEM_OP_MULTIPLE = 2
} mem_op_t;

mem_op_t memory_op_type(uint32_t instr) {
    if (((instr >> 26) & 0b11) == 0b01) {
        // single transfer, bits 27:26 = 0b01
        return MEM_OP_SINGLE;
    }

    if (((instr >> 25) & 0b111) == 0b100) {
        // multiple transfer, bits 27:25 = 100
        return MEM_OP_MULTIPLE;
    }
    return MEM_OP_NONE;
}

void validate_single_transfer(uint32_t instr, uint32_t *pc, uint32_t *regs) {
    unsigned Rn = (instr >> 16) & 0b1111;
    if (Rn == 15) {
        // pc-relative addressing.  we don't check this.
        return;
    }

    uint32_t *base = (uint32_t *) regs[Rn];

    uint32_t offset = 0;
    uint32_t *access = base + offset;

    if (!ck_ptr_is_alloced(base)) {
        panic("invalid access to unallocated memory: pc=%x, base=%x, offset=%d\n", pc, base, offset);
    }
}

// note: lr = the pc that we were interrupted at.
// longer term: pass in the entire register bank so we can figure
// out more general questions.
void ck_mem_interrupt(uint32_t* pc, uint32_t *regs) {
    // we don't know what the user was doing.

    uint32_t instr = *pc;
    mem_op_t type = memory_op_type(instr);
    if (type == MEM_OP_NONE) {
        return; // not a memory op
    }
    if (type == MEM_OP_SINGLE) {
        // single transfer
        validate_single_transfer(instr, pc, regs);
    }
    if (type == MEM_OP_MULTIPLE) {
        
    }

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
