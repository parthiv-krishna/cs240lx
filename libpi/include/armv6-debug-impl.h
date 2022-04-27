#ifndef __ARMV6_DEBUG_IMPL_H__
#define __ARMV6_DEBUG_IMPL_H__
#include "armv6-debug.h"
#include "rpi-interrupts.h"

// all your code should go here.  implementation of the debug interface.

// example of how to define get and set for status registers
coproc_mk(status, p14, 0, c0, c1, 0)

// you'll need to define these and a bunch of other routines.
// static inline uint32_t cp15_dfsr_get(void);
// static inline uint32_t cp15_ifar_get(void);
// static inline uint32_t cp15_ifsr_get(void);
// static inline uint32_t cp14_dscr_get(void);

// static inline uint32_t cp14_wcr0_set(uint32_t r);



// Debug ID Reg page 13-5
coproc_mk(didr, p14, 0, c0, c0, 0)
// Debug Status and Control Reg page 13-5
coproc_mk(dscr, p14, 0, c0, c1, 0)

// Instruction Fault Status Reg page 3-68
coproc_mk(ifsr, p15, 0, c5, c0, 1) 
// Breakpoint Control Reg 0 page 13-6
coproc_mk(bcr0, p14, 0, c0, c0, 5);
// Instruction Fault Addr Reg page 3-69
coproc_mk(ifar, p15, 0, c6, c0, 2)
// Breakpoint Value Reg 0 page 13-6
coproc_mk(bvr0, p14, 0, c0, c0, 4);

// Data Fault Status Reg page 3-66
coproc_mk(dfsr, p15, 0, c5, c0, 0)
// Watchpoint Control Reg 0 page 13-6
coproc_mk(wcr0, p14, 0, c0, c0, 7)
// Watchpoint Fault Addr Reg page 13-12
coproc_mk(wfar, p14, 0, c0, c6, 0)
// Watchpoint Value Reg 0 page 13-20
coproc_mk(wvr0, p14, 0, c0, c0, 6)
// Fault Addr Reg page 3-68
coproc_mk(far, p15, 0, c6, c0, 0);

// return 1 if enabled, 0 otherwise.  
//    - we wind up reading the status register a bunch:
//      could return its value instead of 1 (since is 
//      non-zero).
static inline int cp14_is_enabled(void) {
    uint32_t dscr = cp14_dscr_get();
    return bit_is_on(dscr, DSCR_ENABLE); // 13-9
}

// enable debug coprocessor 
static inline void cp14_enable(void) {
    // if it's already enabled, just return?
    if(cp14_is_enabled())
        panic("already enabled\n");

    // // setup exception handlers
    // extern unsigned _interrupt_table;
    // int_init_reg(&_interrupt_table);

    // for the core to take a debug exception, monitor debug mode has to be both 
    // selected and enabled --- bit 14 clear and bit 15 set.
    uint32_t dscr = cp14_dscr_get();
    dscr = bit_clr(dscr, DSCR_MODE_SELECT); // 13-9
    dscr = bit_set(dscr, DSCR_ENABLE);  // 13-9
    cp14_dscr_set(dscr);
    prefetch_flush();

    assert(cp14_is_enabled());
}

// disable debug coprocessor
static inline void cp14_disable(void) {
    if(!cp14_is_enabled())
        return;

    uint32_t dscr = cp14_dscr_get();
    dscr = bit_clr(dscr, DSCR_ENABLE);  // 13-9
    cp14_dscr_set(dscr);
    prefetch_flush();

    assert(!cp14_is_enabled());
}


static inline int cp14_bcr0_is_enabled(void) {
    uint32_t bcr0 = cp14_bcr0_get();
    return bit_is_on(bcr0, BREAKPT_CTRL_EN);
}

static inline void cp14_bcr0_enable(void) {
    // 13-45
    
    // read the BCR
    uint32_t bcr0 = cp14_bcr0_get();

    // clear the enable breakpoint bit and write back
    bcr0 = bit_clr(bcr0, BREAKPT_CTRL_EN);
    cp14_bcr0_set(bcr0);

    struct breakpt_ctrl_t *bcr0_ptr = (struct breakpt_ctrl_t *)&bcr0;

    // page 13-18
    bcr0_ptr->meaning = 0b00; // imva
    bcr0_ptr->enable_linking = 0; // no linking
    bcr0_ptr->secure = 0b00; // breakpoints both in secure and non secure 
    bcr0_ptr->byte_addr_sel = 0b1111; // byte address select all addresses
    bcr0_ptr->sv_access = 0b11; // privileged and user
    bcr0_ptr->breakpt_enable = 1;

    cp14_bcr0_set(bcr0);
    prefetch_flush();
}

static inline void cp14_bcr0_disable(void) {
    uint32_t bcr0 = cp14_bcr0_get();
    bcr0 = bit_clr(bcr0, BREAKPT_CTRL_EN); // disable 13-22
    cp14_bcr0_set(bcr0);
    prefetch_flush();
}

// was this a brkpt fault?
static inline int was_brkpt_fault(void) {
    // use IFSR and then DSCR
    
    // confirm a debug exception has occurred
    uint32_t ifsr = cp15_ifsr_get();
    // 3-65
    if (bit_is_on(ifsr, IFSR_S)) {
        return 0;
    }
    uint32_t status = bits_get(ifsr, IFSR_STATUS_LO, IFSR_STATUS_HI);
    if (status != IFSR_INSTR_DEBUG_EVENT_FAULT) {
        return 0;
    }
    // if bit 10 is off and bits 0:3 are 0b0010 then 
    // an instruction debug event fault occurred
    
    // check DSCR bits 2:5 for reason for entering 
    // see page 13-11
    uint32_t dscr = cp14_dscr_get();
    uint32_t reason = bits_get(dscr, DSCR_REASON_LO, DSCR_REASON_HI);
    if (reason == DSCR_BREAKPOINT) {
        return 1;
    }
    return 0;
}

// was watchpoint debug fault caused by a load?
static inline int datafault_from_ld(void) {
    return bit_isset(cp15_dfsr_get(), 11) == 0;
}
// ...  by a store?
static inline int datafault_from_st(void) {
    return !datafault_from_ld();
}


// 13-33: tabl 13-23
static inline int was_watchpt_fault(void) {
    // use DFSR then DSCR
    
    // confirm a debug exception has occurred
    uint32_t dfsr = cp15_dfsr_get();
    // 3-65
    if (bit_is_on(dfsr, DFSR_S)) {
        return 0;
    }
    uint32_t status = bits_get(dfsr, DFSR_STATUS_LO, DFSR_STATUS_HI);
    if (status != DFSR_INSTR_DEBUG_EVENT_FAULT) {
        return 0;
    }
    // if bit 10 is off and bits 0:3 are 0b0010 then 
    // an instruction debug event fault occurred
    
    // check DSCR bits 2:5 for reason for entering 
    // see page 13-11
    uint32_t dscr = cp14_dscr_get();
    uint32_t reason = bits_get(dscr, DSCR_REASON_LO, DSCR_REASON_HI);
    if (reason == DSCR_WATCHPOINT) {
        return 1;
    }
    return 0;
}

static inline int cp14_wcr0_is_enabled(void) {
    uint32_t wcr0 = cp14_wcr0_get();
    return bit_is_on(wcr0, WATCHPT_CTRL_EN);
}

static inline void cp14_wcr0_enable(void) {
    // 13-45
    uint32_t wcr0 = cp14_wcr0_get();

    wcr0 = bit_clr(wcr0, WATCHPT_CTRL_EN);
    cp14_wcr0_set(wcr0);

    struct watchpt_ctrl_t *wcr0_ptr = (struct watchpt_ctrl_t *)&wcr0;

    // 13-21 and 13-22
    wcr0_ptr->enable_linking = 0b0; // no linking
    wcr0_ptr->secure = 0b00; // watchpoints both in secure and non secure 
    wcr0_ptr->byte_addr_sel = 0b1111; // byte address select all accesses
    wcr0_ptr->load_store_access = 0b11; // loads and stores
    wcr0_ptr->sv_access = 0b11; // privileged and user
    wcr0_ptr->watchpt_enable = 0b1; // enable

    cp14_wcr0_set(wcr0);
    prefetch_flush();
}
static inline void cp14_wcr0_disable(void) {
    uint32_t wcr0 = cp14_wcr0_get();
    wcr0 = bit_clr(wcr0, WATCHPT_CTRL_EN); // disable 13-22
    cp14_wcr0_set(wcr0);
    prefetch_flush();
}

// Get watchpoint fault using WFAR
static inline uint32_t watchpt_fault_pc(void) {
    uint32_t wfar = cp14_wfar_get();
    // see 13-12:  WFAR contains the address of the instruction causing it plus 0x8.
    return wfar - 0x8;
}

static inline uint32_t watchpt_fault_addr(void) {
    uint32_t far = cp15_far_get();
    return far;
}
    
#endif
