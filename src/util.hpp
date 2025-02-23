#include <Arduino.h>

// #include <cmath>

#include "fft.h"

#define MAX_NON_ZERO_SAMPLES 384
float windowValues[MAX_NON_ZERO_SAMPLES];
void preprocess_windowing(unsigned int non_zero_samples) {
  for (int i = 0; i < non_zero_samples; i++) {
    double windowValue =
        0.54 - (0.46 * cos(2.0 * PI * i / (non_zero_samples - 1)));
    windowValues[i] = windowValue;
  }
}

float get_fft_resoult(fft_config_t *fft_analysis, int i) {
  float po = pow(fft_analysis->output[(i + 1) * 2], 2) +
             pow(fft_analysis->output[(i + 1) * 2 + 1], 2);

  return sqrt(po);
}