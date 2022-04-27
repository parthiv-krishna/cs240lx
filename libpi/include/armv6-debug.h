#ifndef __ARMV6_DEBUG_H__
#define __ARMV6_DEBUG_H__

/*************************************************************************
 * all the different assembly routines.
 */
#include "asm-helpers.h"

#if 0
// all we need for IMB at the moment: prefetch flush.
static inline void prefetch_flush(void) {
    unsigned r = 0;
    asm volatile ("mcr p15, 0, %0, c7, c5, 4" :: "r" (r));
}
#endif

// turn <x> into a string
#define MK_STR(x) #x

// define a general co-processor inline assembly routine to set the value.
// from manual: must prefetch-flush after each set.
#define coproc_mk_set(fn_name, coproc, opcode_1, Crn, Crm, opcode_2)       \
    static inline void c ## coproc ## _ ## fn_name ## _set(uint32_t v) {                    \
        asm volatile ("mcr " MK_STR(coproc) ", "                        \
                             MK_STR(opcode_1) ", "                      \
                             "%0, "                                     \
                            MK_STR(Crn) ", "                            \
                            MK_STR(Crm) ", "                            \
                            MK_STR(opcode_2) :: "r" (v));               \
        prefetch_flush();                                               \
    }

#define coproc_mk_get(fn_name, coproc, opcode_1, Crn, Crm, opcode_2)       \
    static inline uint32_t c ## coproc ## _ ## fn_name ## _get(void) {                      \
        uint32_t ret=0;                                                   \
        asm volatile ("mrc " MK_STR(coproc) ", "                        \
                             MK_STR(opcode_1) ", "                      \
                             "%0, "                                     \
                            MK_STR(Crn) ", "                            \
                            MK_STR(Crm) ", "                            \
                            MK_STR(opcode_2) : "=r" (ret));             \
        return ret;                                                     \
    }


// make both get and set methods.
#define coproc_mk(fn, coproc, opcode_1, Crn, Crm, opcode_2)     \
    coproc_mk_set(fn, coproc, opcode_1, Crn, Crm, opcode_2)        \
    coproc_mk_get(fn, coproc, opcode_1, Crn, Crm, opcode_2) 

// produces p14_brv_get and p14_brv_set
// coproc_mk(brv, p14, 0, c0, crm, op2)

/*******************************************************************************
 * debug support.
 */
#include "libc/helper-macros.h"     // to check the debug layout.
#include "libc/bit-support.h"           // bit_* and bits_* routines.


// 13-5
struct debug_id {
                                // lower bit pos : upper bit pos [inclusive]
                                // see 0-example-debug.c for how to use macros
                                // to check bitposition and size.  very very easy
                                // to mess up: you should always do.
    uint32_t    revision:4,     // 0:3  revision number
                variant:4,      // 4:7  major revision number
                :4,             // 8:11
                debug_rev:4,   // 12:15
                debug_ver:4,    // 16:19
                context:4,      // 20:23
                brp:4,          // 24:27 --- number of breakpoint register
                                //           pairs+1
                wrp:4          // 28:31 --- number of watchpoint pairs.
        ;
};

// 13-8
enum debug_status {
    DSCR_REASON_LO = 2,    // bits 2-5 are reason for jumping to prefetch/data abort
    DSCR_REASON_HI = 5,
    DSCR_MODE_SELECT = 14,  // 14 mode select bit
    DSCR_ENABLE = 15, // 15 monitor debug mode enable

    // 13-11: methods of entering prefetch/data abort vector
    DSCR_BREAKPOINT = 0b0001,
    DSCR_WATCHPOINT = 0b0010,
};

// 3-67
enum inst_fault_status {
    IFSR_STATUS_LO = 0, // low bit of status
    IFSR_STATUS_HI = 3, // hi bit of status
    IFSR_S = 10, // 10 S bit
    
    IFSR_INSTR_DEBUG_EVENT_FAULT = 0b0010 // bottom 4 bits for debug event
};

// 13-18 and 13-19
enum breakpt_ctrl {
    BREAKPT_CTRL_EN = 0,   // 0 enable bit
};

// 13-18 and 13-19
struct breakpt_ctrl_t {
    uint32_t breakpt_enable:1,  // 0 breakpt enable
             sv_access:2,       // 1:2 supervisor access
             :2,                // 3:4 reserved
             byte_addr_sel:4,   // 5:8 byte address select
             :5,                // 9:13 reserved
             secure:2,          // 14:15 matching in secure/not secure worlds
             brp:4,             // 16:19 linked breakpoint
             enable_linking:1,  // 20 enable linking
             meaning:2,         // meaining of BVR 
             :9                 // 23:31 reserved
             ;
};

// 3-65
enum data_fault_status {
    DFSR_STATUS_LO = 0, // low bit of status
    DFSR_STATUS_HI = 3, // hi bit of status
    DFSR_S = 10, // 10 S bit
    
    DFSR_INSTR_DEBUG_EVENT_FAULT = 0b0010 // bottom 4 bits for debug event
};

// 13-21 and 13-22
enum watchpt_ctrl {
    WATCHPT_CTRL_EN = 0,   // 0 enable bit
};

// 13-21 and 13-22
struct watchpt_ctrl_t {
    uint32_t watchpt_enable:1,  // 0 watchpoint enable
             sv_access:2,       // 1:2 supervisor access
             load_store_access:2, // 3:4 load/store access conditioning
             byte_addr_sel:4,   // 5:8 byte address select
             :5,                // 9:13 reserved
             secure:2,          // 14:15 matching in secure/not secure worlds
             brp:4,             // 16:19 linked breakpoint
             enable_linking:1,  // 20 enable linking
             :11                // 21:31 reserved
             ;
};


// Get the debug id register
static inline uint32_t cp14_debug_id_get(void) {
    // the documents seem to imply the general purpose register 
    // SBZ ("should be zero") so we clear it first.
    uint32_t ret = 0;

    asm volatile ("mrc p14, 0, %0, c0, c0, 0" : "=r"(ret));
    return ret;
}

// This macro invocation creates a routine called cp14_debug_id_macro
// that is equivalant to <cp14_debug_id_get>
//
// you can see this by adding "-E" to the gcc compile line and inspecting
// the output.
coproc_mk_get(debug_id_macro, p14, 0, c0, c0, 0)

// enable the debug coproc
static inline void cp14_enable(void);

// get the cp14 status register.
static inline uint32_t cp14_status_get(void);
// set the cp14 status register.
static inline void cp14_status_set(uint32_t status);

static inline uint32_t cp15_dfsr_get(void);
static inline uint32_t cp15_ifar_get(void);
static inline uint32_t cp15_ifsr_get(void);
static inline uint32_t cp14_dscr_get(void);

#endif
