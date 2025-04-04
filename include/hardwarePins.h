#pragma once

// i2s audio in
#define I2S_NUM I2S_NUM_0
#define I2S_BCK_IO 2   // Bit Clock (BCK) pin on ESP32
#define I2S_WS_IO 32   // Word Select (LRCK) pin on ESP32
#define I2S_DO_IO 33   // Data Output (DOUT) pin from PCM1808
#define I2S_MCLK_IO 0  // SCKI system clock output pin

// hub75
#define R1_PIN_DEFAULT 25
#define G1_PIN_DEFAULT 26
#define B1_PIN_DEFAULT 27
#define R2_PIN_DEFAULT 14
#define G2_PIN_DEFAULT 12
#define B2_PIN_DEFAULT 13

#define A_PIN_DEFAULT 23
#define B_PIN_DEFAULT 19
#define C_PIN_DEFAULT 5
#define D_PIN_DEFAULT 17
#define E_PIN_DEFAULT \
  -1  // IMPORTANT: Change to a valid pin if using a 64x64px panel.

#define LAT_PIN_DEFAULT 4
#define OE_PIN_DEFAULT 15
#define CLK_PIN_DEFAULT 16
