#include "rpi.h"
#include "fft.h"
#include "i2s.h"
#include "neopixel.h"

#define LOG2_FFT_LEN 10
#define FFT_LEN (1 << LOG2_FFT_LEN)

#define FS 44100

#define NEOPIX_PIN 2
#define NEOPIX_LEN 16
#define NEOPIX_MIN_FREQ 40
#define NEOPIX_MAX_FREQ 4000
#define NEOPIX_DIV 32

#define START_IDX ((NEOPIX_MIN_FREQ * FFT_LEN)/ FS)
#define FREQS_PER_BUCKET ((NEOPIX_MAX_FREQ - NEOPIX_MIN_FREQ) / (NEOPIX_LEN) / (FS / FFT_LEN))

int16_t to_q15(uint32_t x) {
    int32_t xs = x - (1 << 31);
    return (int16_t)(xs >> 10);
}

void notmain(void) {
    enable_cache();
    i2s_init();
    neo_t neo = neopix_init(NEOPIX_PIN, NEOPIX_LEN);

    int16_t real[FFT_LEN] = {0};
    int16_t imag[FFT_LEN] = {0};

    while (1) {
        for (int i = 0; i < FFT_LEN; i++) {
            real[i] = to_q15(i2s_read_sample());
            imag[i] = 0;
        }

        fft_fixed_cfft(real, imag, LOG2_FFT_LEN, 0);

        int16_t data_max = 0;
        int16_t data_max_idx = 0;

        for (int i = 0; i < NEOPIX_LEN; i++) {
            int32_t acc = 0;
            for (int j = 0; j < FREQS_PER_BUCKET; j++) {
                int fft_idx = START_IDX + (FREQS_PER_BUCKET*i) + j;
                acc += fft_fixed_mul_q15(real[i], real[i]) + fft_fixed_mul_q15(imag[i], imag[i]);
            }
            int val = (acc > 0xFF * NEOPIX_DIV) ? 0xFF : acc / NEOPIX_DIV;
            neopix_write(neo, i, val, val, val);
        }
        neopix_flush(neo);
    }

    clean_reboot();

}