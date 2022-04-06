#include "rpi.h"
#include "../unix-side/armv6-insts.h"

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#include "cycle-util.h"

#define MAX_BYTES 1024

#define USE_CACHE 1
#if USE_CACHE
    #define RESET_CACHE() printk("resetting cache\n"); flush_all_caches(); \
                          disable_cache(); enable_cache()
#else
    #define RESET_CACHE() printk("not using cache\n")
#endif

typedef void (*int_fp)(void);

static volatile unsigned cnt = 0;

// fake little "interrupt" handlers: useful just for measurement.
void int_0() { cnt++; }
void int_1() { cnt++; }
void int_2() { cnt++; }
void int_3() { cnt++; }
void int_4() { cnt++; }
void int_5() { cnt++; }
void int_6() { cnt++; }
void int_7() { cnt++;}

void generic_call_int(int_fp *intv, unsigned n) { 
    for(unsigned i = 0; i < n; i++)
        intv[i]();
}

// you will generate this dynamically.
void specialized_call_int(void) {
    int_0();
    int_1();
    int_2();
    int_3();
    int_4();
    int_5();
    int_6();
    int_7();
}

void rewrite_one(int_fp to_rewrite, int_fp target) {
    uint32_t *curr = (uint32_t *)to_rewrite;
    uint32_t bxlr = arm_bx(arm_lr);
    while (*curr != bxlr) {
        curr++;
        if (curr - (uint32_t *)to_rewrite >= MAX_BYTES) {
            panic("infinite loop");
        }
    }

    *curr = arm_b((uint32_t)curr, (uint32_t)target);
}

void notmain(void) {

    int_fp intv[] = {
        int_0,
        int_1,
        int_2,
        int_3,
        int_4,
        int_5,
        int_6,
        int_7
    };

    cycle_cnt_init();

    unsigned n = NELEM(intv);

    // try with and without cache: but if you modify the routines to do 
    // jump-threadig, must either:
    //  1. generate code when cache is off.
    //  2. invalidate cache before use.
    RESET_CACHE();

    cnt = 0;
    TIME_CYC_PRINT10("cost of generic-int calling",  generic_call_int(intv,n));
    demand(cnt == n*10, "cnt=%d, expected=%d\n", cnt, n*10);

    // flush to make it fair
    RESET_CACHE();

    // rewrite to generate specialized caller dynamically.
    cnt = 0;
    TIME_CYC_PRINT10("cost of specialized int calling", specialized_call_int() );
    demand(cnt == n*10, "cnt=%d, expected=%d\n", cnt, n*10);

    // rewrite 
    for (int i = 0; i < n - 1; i++) {
        rewrite_one(intv[i], intv[i + 1]);
    }

    // need to flush
    RESET_CACHE();

    cnt = 0;
    TIME_CYC_PRINT10("cost of jump-threaded", int_0() );
    demand(cnt == n*10, "cnt=%d, expected=%d\n", cnt, n*10);

    clean_reboot();
}
