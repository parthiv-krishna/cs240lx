/*
 * simple test to use your buffered neopixel interface to push a cursor around
 * a light array.
 */
#include "rpi.h"
#include "WS2812B.h"
#include "neopixel.h"
#include "i2s.h"

// the pin used to control the light strip.
enum { pix_pin = 2 };

// crude routine to write a pixel at a given location.
void place_cursor(neo_t h, int i) {
    neopix_write(h,i-2,0xff,0,0);
    neopix_write(h,i-1,0,0xff,0);
    neopix_write(h,i,0,0,0xff);
    neopix_flush(h);
}

int32_t abs(int32_t x) {
    return x < 0 ? -x : x;
}

void notmain(void) {
    enable_cache(); 
    gpio_set_output(pix_pin);

    // make sure when you implement the neopixel 
    // interface works and pushes a pixel around your light
    // array.
    unsigned npixels = 16;  // you'll have to figure this out.
    neo_t h = neopix_init(pix_pin, npixels);

    i2s_init();

    uint32_t last = 0;

    for(int j = 0; j < 100000; j++) {
        
        // really dumb way of generating some number to attempt
        // to quantify power spectral density. the proper way
        // would be some sort of FFT        
        uint32_t tot = 0;
        for (int i = 0; i < 128; i++) {
            int32_t curr = i2s_read_sample();
            int32_t diff = curr - last;
            tot += abs(diff) >> 22;
            last = curr;
        }
        place_cursor(h, tot / 128);
    }
    output("done!\n");
}
