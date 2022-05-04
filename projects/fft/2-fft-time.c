#include "rpi.h"
#include "fft.h"

#define LOG2_N 8
#define N (1 << LOG2_N)

#define NUM_TRIALS 1024

void notmain(void) {

    int16_t data[N];

    for (int i = 0; i < N; i++) {
        data[i] = 1024;
    }


    uint32_t start = timer_get_usec();
    for (int i = 0; i < NUM_TRIALS; i++) {
        fft_fixed_rfft(data, LOG2_N, 0);
    }
    uint32_t end = timer_get_usec();

    printk("%dus per %d-point FFT\n", (end - start) / NUM_TRIALS, N);

    clean_reboot();
}