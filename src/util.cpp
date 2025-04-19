// #include <cmath>
#include "util.h"

#include <Arduino.h>

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

// Returns PDF of normal distribution with max value of 1
float normalPDF(float x) { return exp(-4 * x * x); }

float flatPDF(float x) {
  x = x * x;
  return exp(-4 * x * x);
}

// idea is to to center pdf that has most cululative probability in [-1,1]
float centerPDF(int x, int max_x, std::function<float(float)> PDF) {
  return PDF((1.0 * x / (max_x / 2)) - 1);
}