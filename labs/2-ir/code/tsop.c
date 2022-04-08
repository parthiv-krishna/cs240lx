// very dumb starter code.  you should rewrite and customize.
//
// when done i would suggest pulling it out into an device source/header 
// file and put in libpi/src so can use later.

#include "rpi.h"
enum { ir_eps = 200, 
       timeout_us = 20000,
       delay_0 = 4500, 
       delay_1 = 4500 
    };

// we should never get this.
enum { NOISE = 0 } ;

struct readings { unsigned usec, v; };

const char *key_to_str(unsigned x) {
    output("convert an input value to a string\n");
    unimplemented();
}

// adapt your read_while_equal: return 0 if timeout passed, otherwise
// the number of microseconds + 1 (to prevent 0).
static int read_while_eq(int pin, int v, unsigned timeout) {
    unsigned start = timer_get_usec();
    while (gpio_read(pin) == v) {
        if (timer_get_usec() - start > timeout) {
            return 0;
        }
    }
    unsigned end = timer_get_usec();
    return end - start;
}

// integer absolute value.
static int abs(int x) {
    return x < 0 ? -x : x; 
}

// return 0 if e is closer to <lb>, 1 if its closer to <ub>
static int pick(struct readings *e, unsigned lb, unsigned ub) {
    unimplemented();
    panic("invalid time: <%d> expected %d or %d\n", e->usec, lb, ub);
}

// return 1 if is a skip: skip = delay of 550-/+eps
static int is_skip(struct readings *e) {
    unimplemented();
    return 0;
}

// header is a delay of 9000 and then a delay of 4500
int is_header(struct readings *r, unsigned n) {
    if(n < 2)
        return 0;
    int zero = abs(r[0].usec - delay_0) <= ir_eps;
    int one = abs(r[1].usec - delay_1) <= ir_eps;
    return (zero && one);
}

// convert <r> into an integer by or'ing in 0 or 1 depending on the 
// time value.
//
// assert that they are seperated by skips!
unsigned convert(struct readings *r, unsigned n) {
    unimplemented();
}

static void print_readings(struct readings *r, int n) {
    assert(n);
    printk("-------------------------------------------------------\n");
    for(int i = 0; i < n; i++) {
        if(i) 
            assert(!is_header(r+i,n-i));
        printk("\t%d: %d = %d usec\n", i, r[i].v, r[i].usec);
    }
    printk("readings=%d\n", n);
    if(!is_header(r,n))
        printk("NOISE\n");
    else
        printk("convert=%x\n", convert(r,n));
}

// read in values until we get a timeout, return the number of readings.  
static int get_readings(int in, struct readings *r, unsigned N) {
    int n = 0;
    while (1) {
        int curr = gpio_read(in);
        int v = read_while_eq(in, curr, timeout_us);
        if (v == 0) {
            break;
        }
        if (n == N) {
            panic("too many readings\n");
        }
        r[n].usec = v;
        r[n++].v = curr;
    }
    return n;
}

// initialize the pin.
int tsop_init(int input) {
    // is open hi or lo?  have to set pullup or pulldown
    gpio_set_input(input);
    gpio_set_pullup(input);
    return 1;
}

void notmain(void) {
    int in = 21;
    tsop_init(in);
    output("about to start reading\n");

    // very dumb starter code
    while(1) {
        // wait until signal: or should we timeout?
        while(gpio_read(in))
            ;
#       define N 256
        static struct readings r[N];
        int n = get_readings(in, r, N);

        output("done getting readings\n");
        print_readings(r, n);
    
        unsigned x = convert(r,n);
        output("converted to %x\n", x);
        const char *key = key_to_str(x);
        if(key)
            printk("%s\n", key);
        else
            // failed: dump out the data so we can figure out what happened.
            print_readings(r,n);
    }
	printk("stopping ir send/rec!\n");
    clean_reboot();
}
