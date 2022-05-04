#include "rpi.h"
#include "fft.h"

#define LOG2_N 4
#define N (1 << LOG2_N)

void notmain(void) {
    enable_cache();

    int16_t data[N] = {1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024,1024};

    printk("IN\n");
    for (int i = 0; i < N; i++) {
        printk("%d ", data[i]);
    }

    printk("\nFWD\n");
    fft_fixed_rfft(data, LOG2_N , 0);

    for (int i = 0; i < N; i++) {
        printk("%d ", data[i]);
    }

    printk("\nINV\n");
    fft_fixed_rfft(data, LOG2_N , 1);

    for (int i = 0; i < N; i++) {
        printk("%d ", data[i]);
    }
    printk("\n");

    clean_reboot();
}