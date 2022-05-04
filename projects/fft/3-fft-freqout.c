#include "rpi.h"
#include "fft.h"
#include "i2s.h"

#define FFT_LEN 1024
#define LOG2_FFT_LEN 10
#define FS 44100
// attempt to reject harmonics. 
#define MAX_THRESH_FACTOR 5 / 4

int16_t to_q15(uint32_t x) {
    int32_t xs = x - (1 << 31);
    return (int16_t)(xs >> 12);
}

void notmain(void) {
    enable_cache();
    i2s_init();

    int16_t real[FFT_LEN] = {0};
    int16_t imag[FFT_LEN] = {0};

    int i = 0;
    while (i < 2) {
        i = 1;
        for (int i = 0; i < FFT_LEN; i++) {
            real[i] = to_q15(i2s_read_sample());
            imag[i] = 0;
        }

        fft_fixed_cfft(real, imag, LOG2_FFT_LEN, 0);

        int16_t data_max = 0;
        int16_t data_max_idx = 0;

        for (int i = 0; i < FFT_LEN; i++) {
            int32_t mag = fft_fixed_mul_q15(real[i], real[i]) + fft_fixed_mul_q15(imag[i], imag[i]);
            if (mag > data_max * MAX_THRESH_FACTOR) {
                data_max = mag;
                data_max_idx = i;
            }
        }

        int16_t freq = data_max_idx * FS / FFT_LEN;

        printk("%d\n", freq);

    }

    clean_reboot();

}