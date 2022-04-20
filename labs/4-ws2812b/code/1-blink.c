/*
 * simple test to turn on the first pixel in the light string to blue: if your code
 * is correct, it will just work.  you should modify to change pixel color and which
 * pixel goes on.
 */
#include "rpi.h"
#include "WS2812B.h"
#include "neopixel.h"

// the pin used to control the light strip.
enum { pix_pin = 21 };

void notmain(void) {
    // if you don't do this, the granularity is too large for the timing
    // loop. 
    enable_cache(); 
    gpio_set_output(pix_pin);

    // turn on one pixel to blue.
    // alter the code to make sure you can:
    //  1. write red and gree
    //  2. write different pixels.
    for(unsigned i = 0; i < 8; i++) {
        output("setting on\n");
        for (int i = 0; i < 4; i++) {
            pix_sendpixel(pix_pin, 0,0,0xff);
            pix_sendpixel(pix_pin, 0,0xff,0);
            pix_sendpixel(pix_pin, 0xff,0,0);
            pix_sendpixel(pix_pin, 0xff, 0xff, 0xff);
        }

        pix_flush(pix_pin);
        delay_ms(1000);

        output("setting off\n");
        for (int i = 0; i < 16; i++) {
            pix_sendpixel(pix_pin, 0,0,0);
        }
        pix_flush(pix_pin);
        delay_ms(1000);
    }
    output("done\n");
}
