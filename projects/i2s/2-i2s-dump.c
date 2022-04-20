/*
 * simple test to see that i2s is working
 */
#include "rpi.h"
#include "i2s.h"

#define N 44100

void notmain(void) {
    enable_cache(); 

    // make sure when you implement the neopixel 
    // interface works and pushes a pixel around your light
    // array.

    int32_t *buf = (int32_t *)kmalloc(N * sizeof(int32_t));

    i2s_init();

    unsigned start = timer_get_usec();
    for (int i = 0; i < N; i++) {
        buf[i] = i2s_read_sample();
    }
    unsigned end = timer_get_usec();

    // for (int i = 0; i < N; i++) {
    //     printk("%d\n", buf[i]);  
    // }

    printk("Measured %d samples in %d us\n", N, end - start);

    output("done!\n");
}
