// "bootloader" code that moves itself out of the way of incoming code so that
// we don't have to make an external mess of special makefiles, relinking, etc.
#include "rpi.h"
#include "cpyjmp.h"

#define cpy_out(args...) do { } while(0)

// copy the memory [<code>, <nbytes>] to <code_dst>.
// must be compiled to position independent code (you will check).
// 
// note:
//  - cannot call an external routine directly since it will be at a 
//    different location.  
//  - similarly cannot use global variables since the pc relative load 
//    won't work
//  - most likely should (but we don't) switch stacks to be safe during 
//    the copy.  this is probably the biggest possible source of bugs atm.
__attribute__((always_inline)) static inline void 
cpyjmp_internal(uint32_t code_dst, const void *code, unsigned nbytes) {
    // copy [code, code+nbytes) to <code_dst> and jump to <code_dst>.
    char *code_char = (char *)code;
    char *code_dst_char = (char *)code_dst;
    for (unsigned i = 0; i < nbytes; i++) {
        code_dst_char[i] = code_char[i];
    }

    cpyjmp_t func = (cpyjmp_t)code_dst;
    func(code_dst, code, nbytes);

    // the jump to <code_dst> should never return.
    // need a better safety net.
    asm volatile ("swi 1");
}


// make two routines that call <cpyjmp_internal> so can check 
// if position independent.  get the size by declaring empty
// non-static routines after.

void cpyjmp(uint32_t code_dst, const void *code, unsigned nbytes) {
    cpyjmp_internal(code_dst,code,nbytes);
}

void cpyjmp_end(void) {
}

void cpyjmp2(uint32_t code_dst, const void *code, unsigned nbytes) {
    cpyjmp_internal(code_dst,code,nbytes);
}

void cpyjmp2_end(void) {
}


// do some simple sanity checks that the generated binary looks as we
// expect.
void cpyjmp_check(void) {
    // check 1: make sure the two routines are the same size.
    int cpyjmp_size = (int *)cpyjmp_end - (int *)cpyjmp;
    int cpyjmp2_size = (int *)cpyjmp2_end - (int *)cpyjmp2;
    assert(cpyjmp_size == cpyjmp2_size);


    // check 2: make sure gcc laid the routines out in order
    //          (the first should have a smaller address than second, etc)
    assert((char *)cpyjmp < (char *)cpyjmp2);

    // check 3: make sure there is "enough" code in the routines (not just
    //          a few instructions.
    assert(cpyjmp_size > 2);

    // check 4: the final important check: make sure both routines are 
    //          identical (memcpy==0).
    for (unsigned i = 0; i < cpyjmp2_size; i++){
        assert(((int *)cpyjmp)[i] == ((int *)cpyjmp2)[i]);
    }
}

// copy <cpyjmp> to location <addr> and return a function pointer to the 
// new location.
cpyjmp_t cpyjmp_relocate(uint32_t addr) {
    assert(addr >= cpyjmp_lowest_reloc_addr);
    cpyjmp_check();

    char *target = (char *)addr;
    for (char *curr = (char *)cpyjmp; curr < (char *)cpyjmp2; curr++, target++) {
        *target = *curr;
    }

    return (cpyjmp_t)addr;
}
