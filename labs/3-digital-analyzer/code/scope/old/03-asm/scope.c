#include "rpi.h"

// cycle counter routines.
#include "cycle-count.h"

#include "scope-asm.h"


// this defines the period: makes it easy to keep track and share
// with the test generator.
#include "../test-gen/test-gen.h"

// trivial logging code to compute the error given a known
// period.
#include "samples.h"

// derive this experimentally: check that your pi is the same!!
#define CYCLE_PER_SEC (700*1000*1000)

// some utility routines: you don't need to use them.
#include "cycle.h"

// implement this code and tune it.
unsigned 
scope(unsigned pin, log_ent_t *l, unsigned n_max, unsigned max_cycles) {
    unsigned v1, v0 = gpio_read(pin);

    // spin until the pin changes.
    while((v1 = gpio_read(pin)) == v0)
        ;

    unsigned rd1 = cycle_cnt_read();
    unsigned rd2 = cycle_cnt_read();
    unsigned read_time = rd2 - rd1;

    // when we started sampling 
    unsigned start = cycle_cnt_read(), t = start;

    // sample until record max samples or until exceed <max_cycles>
    unsigned n = 0;

    // write this code first: record sample when the pin
    // changes.  then start tuning the whole routine.
    // asm volatile (".align 16");

    loop_till_21_high();

    l[n++].ncycles = loop_till_21_low();
    l[n++].ncycles = loop_till_21_high();
    l[n++].ncycles = loop_till_21_low();
    l[n++].ncycles = loop_till_21_high();
    l[n++].ncycles = loop_till_21_low();
    l[n++].ncycles = loop_till_21_high();
    l[n++].ncycles = loop_till_21_low();
    l[n++].ncycles = loop_till_21_high();
    l[n++].ncycles = loop_till_21_low();
    l[n++].ncycles = loop_till_21_high();
    l[n++].ncycles = loop_till_21_low();

    int ofst = l[0].ncycles - start - CYCLE_PER_FLIP - 25;

    for (int i = 0; i < n; i++) {
        l[i].ncycles -= start + ofst;
        l[i].v = v0;
        v0 = 1 - v0;
    }

    printk("timeout! start=%d\n", start);

    return n;
}

void notmain(void) {
    // setup input pin.
    unsigned pin = 21;
    gpio_set_input(pin);
    enable_cache();

    // make sure to init cycle counter hw.
    cycle_cnt_init();

#   define MAXSAMPLES 32
    log_ent_t log[MAXSAMPLES];

    // just to illustrate.  remove this.
    // sample_ex(log, 10, CYCLE_PER_FLIP);

    // run 4 times before rebooting: makes things easier.
    // you can get rid of this.
    for(int i = 0; i < 4; i++) {
        unsigned n = scope(pin, log, MAXSAMPLES, msec_to_cycle(250));
        dump_samples(log, n, CYCLE_PER_FLIP);
    }
    clean_reboot();
}
