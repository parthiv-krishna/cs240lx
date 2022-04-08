#ifndef __VECTOR_BASE_SET_H__
#define __VECTOR_BASE_SET_H__
#include "libc/bit-support.h"
#include "asm-helpers.h"

/*
 * vector base address register:
 *   3-121 --- let's us control where the exception jump table is!
 *
 * defines: 
 *  - vector_base_set  
 *  - vector_base_get
 */

static inline void *vector_base_get(void) {
    void *result = 0;
    asm volatile ("MRC p15, 0, %0, c12, c0, 0" 
                  : "=r" (result) : :);
    return result;
}

// set the vector register to point to <vector_base>.
// must: 
//    - check that it satisfies the alignment restriction.
static inline void vector_base_set(void *vector_base) {
    unsigned int vb = (unsigned int)vector_base;
    if (vb & 0b11111) {
        panic("vector_base must be aligned to 32 bytes!");
    }
    asm volatile ("MCR p15, 0, %0, c12, c0, 0"
                  : : "r" (vector_base) :);

    assert(vector_base_get() == vector_base);
}
#endif
