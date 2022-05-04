#include "rpi.h"
#include "fft.h"
#include "i2s.h"
#include "neopixel.h"

#define FFT_LEN 1024
#define LOG2_FFT_LEN 10
#define FS 44100
#define NBLOCKS 4

#define MAX_VIZ_FREQ 4000
#define NEOPIX_PIN 2
#define NEOPIX_LEN 16


int16_t to_q15(uint32_t x) {
    int32_t xs = x - (1 << 31);
    return (int16_t)(xs >> 10);
}

void notmain(void) {
    enable_cache();
    i2s_init();

    neo_t neo = neopix_init(NEOPIX_PIN, NEOPIX_LEN);
    

    int16_t data[FFT_LEN] = {0};

    while (1) {

        int freq_acc = 0;

        for (int b = 0; b < NBLOCKS; b++) {

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

            int loudest_freq = data_max_idx * FS / FFT_LEN;
            freq_acc += loudest_freq;
        
        }

        freq_acc = freq_acc / NBLOCKS;

        int neo_idx = freq_acc / (MAX_VIZ_FREQ / NEOPIX_LEN); 

        printk("%d, %d\n", freq_acc, neo_idx);

        // neopix_clear(neo);
        neopix_write(neo, neo_idx, 0xFF, 0xFF, 0xFF);
        neopix_flush(neo);

    }

    clean_reboot();

}