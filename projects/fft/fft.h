#ifndef FFT_H
#define FFT_H

#include "rpi.h"

int32_t fft_fixed_cfft(int16_t real[], int16_t imag[], int16_t log2_len, unsigned inverse);
int32_t fft_fixed_rfft(int16_t data[], int32_t log2_len, unsigned inverse);

#endif // FFT_H