#pragma once

//
#include <Arduino.h>

// #include <cmath>

#include "fft.h"

#define MAX_NON_ZERO_SAMPLES 400
extern float windowValues[MAX_NON_ZERO_SAMPLES];

void preprocess_windowing(unsigned int non_zero_samples);

float get_fft_resoult(fft_config_t *fft_analysis, int i);

float centerPDF(int x, int max_x, std::function<float(float)> PDF);
float normalPDF(float x);
float flatPDF(float x);
