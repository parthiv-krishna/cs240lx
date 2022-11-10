#include "rpi.h"
#include "rpi-constants.h"
#include "rpi-interrupts.h"
#include "libc/helper-macros.h"
#include "mmu.h"

/***********************************************************************
 * the following code is given
 */

enum { OneMB = 1024 * 1024 };

// maximum number of 1mb section entries for a 32-bit address space
enum { MAX_SEC_PTE = 4096};

// unsigned next_free_udomain() {
// 	unsigned domID = next_free_u
// }
// unsigned mmu_user_domain_alloc(unsigned ) {
// 	return next_free_udomain();
// }


// disable the mmu by setting control register 1
// to <c:32>.
// 
// we use a C veneer over the assembly (mmu_disable_set_asm)
// so we can easily do assertions: the real work is 
// done by the asm code (you'll write this next time).
void mmu_disable_set(cp15_ctrl_reg1_t c) {
    assert(!c.MMU_enabled);
    assert(!c.C_unified_enable);
	mmu_disable_set_asm(c);
}

// disable the MMU by flipping the enable bit.   we 
// use a C vener so we can do assertions and then call
// out to assembly to do the real work (you'll write this
// next time).
void mmu_disable(void) {
    cp15_ctrl_reg1_t c = cp15_ctrl_reg1_rd();
    assert(c.MMU_enabled);
    c.MMU_enabled=0;
    mmu_disable_set(c);
}

// enable the mmu by setting control reg 1 to
// <c>.   we start in C so we can do assertions
// and then call out to the assembly for the 
// real work (you'll write this code next time).
void mmu_enable_set(cp15_ctrl_reg1_t c) {
    assert(c.MMU_enabled);
    mmu_enable_set_asm(c);
}

// enable mmu by flipping enable bit.
void mmu_enable(void) {
    cp15_ctrl_reg1_t c = cp15_ctrl_reg1_rd();
    assert(!c.MMU_enabled);
    c.MMU_enabled = 1;
    mmu_enable_set(c);
}

// C end of this: does sanity checking then calls asm.
void set_procid_ttbr0(unsigned pid, unsigned asid, void *pt) {
    assert((pid >> 24) == 0);
    assert(pid > 64);
    assert(asid < 64 && asid);
	cp15_set_procid_ttbr0(pid << 8 | asid, pt);
}

// hopefully catch if you call unimplemented stuff.
void asm_not_implemented(unsigned lr) {
    if(mmu_is_enabled())
        mmu_disable();
    panic("called unimplemented asm routine at %x\n", lr);
}


// set so that we use armv6 memory.
// this should be wrapped up neater.  broken down so can replace 
// one by one.
//  1. the fields are defined in vm.h.
//  2. specify armv6 (no subpages).
//  3. check that the coprocessor write succeeded.
void mmu_init(void) { 
    // reset the MMU state: you will implement next lab
    mmu_reset();

    // trivial: RMW the xp bit in control reg 1.
    // leave mmu disabled.
	struct control_reg1 ctrl = cp15_ctrl_reg1_rd();
	ctrl.XP_pt = 1;
	cp15_ctrl_reg1_wr(ctrl);

    // make sure write succeeded.
    struct control_reg1 c1 = cp15_ctrl_reg1_rd();
    assert(c1.XP_pt);
    assert(!c1.MMU_enabled);
}


// read and return the domain access control register
uint32_t domain_access_ctrl_get(void) {
	unsigned ret = 0;
	asm volatile("mrc p15, 0, %0, c3, c0, 0": "=r"(ret) :);
	return ret;
}

// b4-42
// set domain access control register to <r>
void domain_access_ctrl_set(uint32_t r) {
    // staff_domain_access_ctrl_set(r);
	cp15_domain_ctrl_wr(r);
    // assert(domain_access_ctrl_get() == r);
}

void mmu_on_first_time(uint32_t asid, void *null_pt) {
	set_procid_ttbr0(0x140e, asid, null_pt);

	// note: this routine has to flush I/D cache and TLB, BTB, prefetch buffer.
	cp15_ctrl_reg1_t c = cp15_ctrl_reg1_rd();
	c.MMU_enabled = 1;
    mmu_enable_set(c);
    assert(mmu_is_enabled());
}
