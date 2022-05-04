#include "rpi.h"
#include "fft.h"
#include "i2s.h"
#include "neopixel.h"

#define FFT_LEN 1024
#define LOG2_FFT_LEN 10
#define FS 44100
// attempt to reject harmonics. 
#define MAX_THRESH_FACTOR 5 / 4

#define NEOPIX_PIN 2
#define NEOPIX_LEN 16
#define NEOPIX_MIN_FREQ 300
#define NEOPIX_MAX_FREQ 1000


int16_t to_q15(uint32_t x) {
    int32_t xs = x - (1 << 31);
    return (int16_t)(xs >> 12);
}

void notmain(void) {
    enable_cache();
    i2s_init();
    neo_t neo = neopix_init(NEOPIX_PIN, NEOPIX_LEN);

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

        int neopix_idx = freq * NEOPIX_LEN / NEOPIX_MAX_FREQ;

        printk("%dHz, index %d\n", freq, neopix_idx);

        neopix_write(neo, neopix_idx, 0xFF, 0xFF, 0xFF);
        neopix_flush(neo);

    }

    clean_reboot();

}