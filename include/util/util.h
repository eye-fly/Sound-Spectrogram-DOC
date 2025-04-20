#pragma once

//
#include <Arduino.h>

// #include <cmath>

#include "fft.h"

#define MAX_NON_ZERO_SAMPLES 400
extern float windowValues[MAX_NON_ZERO_SAMPLES];

void preprocess_windowing(unsigned int non_zero_samples);

float get_fft_resoult(fft_config_t *fft_analysis, int i);
inline float fast_log_0_9(float x) {
  if (x <= 2.0f) return (x - 1.0f) * 0.693147f;  // log(1+x) ≈ x for x near 0
  if (x <= 4.0f)
    return 0.693147f + (x - 2.0f) * 0.405465f;  // log(2) + log(1 + (x-2)/2)
  if (x <= 8.0f)
    return 1.386294f + (x - 4.0f) * 0.223144f;  // log(4) + log(1 + (x-4)/4)
  return 2.079441f + (x - 8.0f) * 0.117783f;    // log(8) + log(1 + (x-8)/8)
}

float centerPDF(int x, int max_x, std::function<float(float)> PDF);
float normalPDF(float x);
float flatPDF(float x);
