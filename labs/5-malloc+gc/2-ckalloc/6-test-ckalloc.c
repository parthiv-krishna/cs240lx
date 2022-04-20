// trivial test of kr malloc/free: allocates, frees, then 
// reallocates same: makes sure that the base pointer is
// the same each time (so all got freed).
//
// is a weak test.
#include "test-interface.h"

#include "ckalloc.h"

static void *test_alloc_free(unsigned n) {
    uint32_t **ptrs = ckalloc(n * sizeof *ptrs);

    for(int i = 0; i < n; i++) {
        uint32_t *p = ptrs[i] = ckalloc(1);

        // malloc does not guarantee
        // assert(*p == 0);
        *p = i;
        demand(*ptrs[i] == (uint32_t)i, "b[%d]=%d, i=%d\n", i, *ptrs[i], (uint8_t)i);
    }

    for(int i = 0; i < n; i++)
        assert(*ptrs[i] == (uint32_t)i);

    for(int i = 0; i < n; i++)
        ckfree(ptrs[i]);
    ckfree(ptrs);

    return ptrs;
}

void notmain(void) {
    enum { OneMB = 1024 * 1024 };

    unsigned n = 20;

    void *p0 = test_alloc_free(n);
    int ntests = 10;
    for(int i = 0; i < ntests; i++) {
        void *p1 = test_alloc_free(n);

        // test that everything got freed.
        assert(p0 == p1);
        trace("test iter %d passed\n", i);
    }
    trace("success: malloc/free appear to work!\n");
}
