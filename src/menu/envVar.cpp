#include "envVar.h"

volatile int non_zero_samples = 217;  // 111
volatile int volue_adjustment = 25;
volatile int use_log_scale = 3;
volatile int display_max_f = 650;  // FFT_MAX_F;

volatile int falame_colour_enable = 1;
volatile int flame_gradient_len = 8;
volatile int miror = 0;

volatile int enable_voc_channel = 0;
// unsigned int zero_padding = SAMPLES - non_zero_samples;

// double vReal[SAMPLES]; // Array for storing real part of the signal
// double vImag[SAMPLES];
float fftOut[3][CHANNEL_NUMBER][SAMPLES / 2];
int fftOut_reading = -1;
int fftOut_avaiable = 0;

int32_t i2sBuffer[DMA_BUF_LEN * 2];

int16_t serial_i = 0;