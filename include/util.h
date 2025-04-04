#pragma once

//

// #include <cmath>

#include "fft.h"

#define MAX_NON_ZERO_SAMPLES 384
extern float windowValues[MAX_NON_ZERO_SAMPLES];

void preprocess_windowing(unsigned int non_zero_samples);

float get_fft_resoult(fft_config_t *fft_analysis, int i);

float normalPDF(float x);