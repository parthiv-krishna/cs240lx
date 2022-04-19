/*
 * simple test to see that i2s is working
 */
#include "rpi.h"
#include "i2s.h"


void notmain(void) {
    enable_cache(); 

    // make sure when you implement the neopixel 
    // interface works and pushes a pixel around your light
    // array.


    i2s_init();

    for (int i = 0; i < 100000; i++) {
        int32_t sample = i2s_read_sample();
        printk("%x %d\n", sample, sample);  
    }

    output("done!\n");
}
