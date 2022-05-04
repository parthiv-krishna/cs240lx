#include "rpi.h"
#include "fft.h"
#include "i2s.h"

#define FFT_LEN 1024
#define LOG2_FFT_LEN 10
#define FS 44100

int16_t to_q15(uint32_t x) {
    int32_t xs = x - (1 << 31);
    return (int16_t)(xs >> 10);
}

void notmain(void) {
    enable_cache();
    i2s_init();

    int16_t data[FFT_LEN] = {0};

    while (1) {
        for (int i = 0; i < FFT_LEN; i++) {
            data[i] = to_q15(i2s_read_sample());
        }

        fft_fixed_rfft(data, LOG2_FFT_LEN, 0);

        int16_t data_max = 0;
        int16_t data_max_idx = 0;

        for (int i = 0; i < FFT_LEN / 2; i++) {
            int16_t mag = fft_fixed_mul(data[i], data[i]) + fft_fixed_mul(data[i + FFT_LEN / 2], data[i + FFT_LEN / 2]);
            if (mag > data_max) {
                data_max = mag;
                data_max_idx = i;
            }
        }

        printk("%d\n", data_max_idx * FS / FFT_LEN);

    }

    clean_reboot();

}