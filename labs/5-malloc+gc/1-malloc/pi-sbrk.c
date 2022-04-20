#include "kr-malloc.h"

// implement in libpi.
void *sbrk(long increment) {
    if (increment <= 0) {
        panic("sbrk: increment <= 0");
    }
    void *result = kmalloc(increment);
    
    return result;
}
