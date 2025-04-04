#pragma once
#include <Arduino.h>

#define MULT 2
#define CHANNEL_NUMBER 2
#define SAMPLE_RATE 10000  // 1 kHz sample rate
#define DOWNSAMPLE_RATE \
  (350 * MULT * 2)  // 625    // Target sample rate (downsampled)
#define FFT_MAX_F (350 * MULT)
#define DOWNSAMPLE_FACTOR \
  (SAMPLE_RATE / DOWNSAMPLE_RATE)  // Ratio for downsampling (5)
#define BUFFER_SIZE \
  (100)  //*10                                // Buffer size for right channel
         // data
#define FILTER_ORDER 2  // Order of the low-pass filter (adjustable)

#define DMA_BUF_LEN \
  BUFFER_SIZE  // TODO delete                     // DMA buffer length (8 * 16)

// FFT
#define SAMPLES \
  (128 * MULT * 2)  //(256) // Number of samples (must be a power of 2)
// #define ZERO_PADDING (145 + 256)  //(128+256)
// #define WINNDOWING_RATIO 1.0
// #define DCOFFSET 30420.0
#define SCALEDOWN ((2 << 20) * MULT)

extern volatile int non_zero_samples;  // 111
#define DEFAULT_VOLUME_ADJUSTMENT 50
extern volatile int volue_adjustment;
extern volatile int use_log_scale;
extern volatile int display_max_f;  // FFT_MAX_F;

extern volatile int falame_colour_enable;
extern volatile int flame_gradient_len;
extern volatile int miror;

extern volatile int enable_voc_channel;
// unsigned int zero_padding = SAMPLES - non_zero_samples;

// double vReal[SAMPLES]; // Array for storing real part of the signal
// double vImag[SAMPLES];
extern float fftOut[3][CHANNEL_NUMBER][SAMPLES / 2];
extern int fftOut_reading;
extern int fftOut_avaiable;

extern int32_t i2sBuffer[DMA_BUF_LEN * 2];

extern int16_t serial_i;