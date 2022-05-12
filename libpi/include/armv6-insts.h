#ifndef __ARMV6_ENCODINGS_H__
#define __ARMV6_ENCODINGS_H__
// engler, cs240lx: simplistic instruction encodings for r/pi ARMV6.
// this will compile both on our bare-metal r/pi and unix.

#include "rpi.h"

// bit better type checking to use enums.
enum {
    arm_r0 = 0, 
    arm_r1, 
    arm_r2,
    arm_r3,
    arm_r4,
    arm_r5,
    arm_r6,
    arm_r7,
    arm_r8,
    arm_r9,
    arm_r10,
    arm_r11,
    arm_r12,
    arm_r13,
    arm_r14,
    arm_r15,
    arm_sp = arm_r13,
    arm_lr = arm_r14,
    arm_pc = arm_r15
};
_Static_assert(arm_r15 == 15, "bad enum");


// condition code.
enum {
    arm_EQ = 0,
    arm_NE,
    arm_CS,
    arm_CC,
    arm_MI,
    arm_PL,
    arm_VS,
    arm_VC,
    arm_HI,
    arm_LS,
    arm_GE,
    arm_LT,
    arm_GT,
    arm_LE,
    arm_AL,
};
_Static_assert(arm_AL == 0b1110, "bad enum list");

// data processing ops.
enum {
    arm_and_op = 0, 
    arm_eor_op,
    arm_sub_op,
    arm_rsb_op,
    arm_add_op,
    arm_adc_op,
    arm_sbc_op,
    arm_rsc_op,
    arm_tst_op,
    arm_teq_op,
    arm_cmp_op,
    arm_cmn_op,
    arm_orr_op,
    arm_mov_op,
    arm_bic_op,
    arm_mvn_op,
};
_Static_assert(arm_mvn_op == 0b1111, "bad num list");

/************************************************************
 * instruction encodings.  should do:
 *      bx lr
 *      ld *
 *      st *
 *      cmp
 *      branch
 *      alu 
 *      alu_imm
 *      jump
 *      call
 */

// armv6.pdf page A5-3
typedef struct {
    uint32_t imm_or_Rm:12,      // 0-11
             Rd:4,              // 12-15
             Rn:4,              // 16-19
             S:1,               // 20
             opcode:4,          // 21-24
             I:1,               // 25
             sbz:2,             // 26-27
             cond:4;            // 28-31
} arm_dataproc_t;
_Static_assert(sizeof(arm_dataproc_t) == sizeof(uint32_t), "arm_dataproc_t wrong size");

// armv6.pdf page A4-20
typedef struct {
    uint32_t Rm:4, 
             one:4,
             sbo:12,
             bx_opcode:8,
             cond:4;
} arm_bx_t;
_Static_assert(sizeof(arm_bx_t) == sizeof(uint32_t), "arm_bx_t wrong size");


// add instruction:
//      add rdst, rs1, rs2
//  - general add instruction: page A4-6 [armv6.pdf]
//  - shift operatnd: page A5-8 [armv6.pdf]
//
// we do not do any carries, so S = 0.
// static inline unsigned arm_add(uint8_t rd, uint8_t rs1, uint8_t rs2) {
//     assert(arm_add_op == 0b0100);
    
//     // armv6.pdf page A5-3
//     arm_dataproc_t inst;
//     inst.cond = arm_AL;
//     inst.sbz = 0;
//     inst.I = 0;
//     inst.opcode = arm_add_op;
//     inst.S = 0;
//     inst.Rn = rs1;
//     inst.Rd = rd;
//     inst.imm_or_Rm = rs2;

//     return *(unsigned *)&inst;
// }

// c generated
// static int arm_add(uint32_t dst, uint32_t src1, uint32_t src2) {
//     return 0xe0800000 | (dst << 12) | (src1 << 16) | (src2 << 0);
// }

// python generated
// static uint32_t arm_add(uint32_t dst, uint32_t src1, uint32_t src2) {
//     return 0xe0800000 |
//            (dst << 12) |
//            (src1 << 16) |
//            (src2 << 0);
// }

// // <add> of an immediate
// static inline uint32_t arm_add_imm8(uint8_t rd, uint8_t rs1, uint8_t imm) {
//     assert(arm_add_op == 0b0100);
    
//     // armv6.pdf page A5-3
//     arm_dataproc_t inst;
//     inst.cond = arm_AL;
//     inst.sbz = 0;
//     inst.I = 1;
//     inst.opcode = arm_add_op;
//     inst.S = 0;
//     inst.Rn = rs1;
//     inst.Rd = rd;
//     inst.imm_or_Rm = imm;

//     return *(unsigned *)&inst;
// }

    // static inline uint32_t arm_bx(uint8_t reg) {
        
    //     // armv6.pdf page A4-20
    //     arm_bx_t inst;
    //     inst.cond = arm_AL;
    //     inst.bx_opcode = 0b00010010;
    //     inst.sbo = 0xFFF;
    //     inst.one = 1;
    //     inst.Rm = reg;

    //     return *(unsigned *)&inst;
    // }

// load an or immediate and rotate it.
// static inline uint32_t 
// arm_or_imm_rot(uint8_t rd, uint8_t rs1, uint8_t imm8, uint8_t rot_nbits) {
//     unimplemented();
// }

static inline uint32_t arm_add(uint32_t dst, uint32_t src1, uint32_t src2) {
    return 0xe0800000 |
           (dst << 12) |
           (src1 << 16) |
           (src2 << 0);
}

static inline uint32_t arm_add_imm(uint32_t dst, uint32_t src, uint32_t imm) {
    return 0xe2800000 |
           (dst << 12) |
           (src << 16) |
           (imm << 0);
}

static inline uint32_t arm_add_lsl(uint32_t dst, uint32_t src1, uint32_t src2, uint32_t shift) {
    return 0xe0800000 |
           (dst << 12) |
           (src1 << 16) |
           (src2 << 0) |
           (shift << 7);
}

static inline uint32_t arm_sub(uint32_t dst, uint32_t src1, uint32_t src2) {
    return 0xe0400000 |
           (dst << 12) |
           (src1 << 16) |
           (src2 << 0);
}

static inline uint32_t arm_sub_imm(uint32_t dst, uint32_t src, uint32_t imm) {
    return 0xe2400000 |
           (dst << 12) |
           (src << 16) |
           (imm << 0);
}

static inline uint32_t arm_and(uint32_t dst, uint32_t src1, uint32_t src2) {
    return 0xe0000000 |
           (dst << 12) |
           (src1 << 16) |
           (src2 << 0);
}

static inline uint32_t arm_or(uint32_t dst, uint32_t src1, uint32_t src2) {
    return 0xe1800000 |
           (dst << 12) |
           (src1 << 16) |
           (src2 << 0);
}

static inline uint32_t arm_mov_reg(uint32_t dst, uint32_t src) {
    return 0xe1a00000 |
           (dst << 12) |
           (src << 0);
}

static inline uint32_t arm_mov_imm(uint32_t dst, uint32_t imm) {
    return 0xe3a00000 |
           (dst << 12) |
           (imm << 0);
}

static inline uint32_t arm_ldr_no_off(uint32_t dst, uint32_t addr) {
    return 0xe5900000 |
           (dst << 12) |
           (addr << 16);
}

static inline uint32_t arm_ldr_imm_off(uint32_t dst, uint32_t addr, uint32_t offset) {
    return 0xe5900000 |
           (dst << 12) |
           (addr << 16) |
           (offset << 0);
}

static inline uint32_t arm_str_no_off(uint32_t src, uint32_t addr) {
    return 0xe5800000 |
           (src << 12) |
           (addr << 16);
}

static inline uint32_t arm_str_imm_off(uint32_t src, uint32_t addr, uint32_t offset) {
    return 0xe5800000 |
           (src << 12) |
           (addr << 16) |
           (offset << 0);
}

static inline uint32_t arm_nop() {
    return 0xe320f000;
}

static inline uint32_t arm_bx(uint32_t reg) {
    return 0xe12fff10 |
           (reg << 0);
}

static inline uint32_t arm_blx(uint32_t reg) {
    return 0xe12fff30 |
           (reg << 0);
}


static inline uint32_t arm_b(uint32_t current, uint32_t target) {
    int32_t offset = target - (current + 8); // pc + 8 nonsense
    uint32_t imm = (offset >> 2) & 0xFFFFFF;
    return 0xea000000 |
           (imm << 0);
} 

static inline uint32_t arm_bl(uint32_t current, uint32_t target) {
    int32_t offset = target - (current + 8); // pc + 8 nonsense
    uint32_t imm = (offset >> 2) & 0xFFFFFF;
    return 0xeb000000 |
           (imm << 0);
}



#endif
