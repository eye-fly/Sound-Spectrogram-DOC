#pragma once

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include <stdint.h>

#define BCK_H 64
#define BCK_W (128 * 2)

#define HALFPOINT_WIGGLE 20

// Display
#define PANEL_WIDTH \
  128  // Number of pixels wide of each INDIVIDUAL panel module.
#define PANEL_HEIGHT \
  64  // Number of pixels tall of each INDIVIDUAL panel module.
#define PANELS_NUMBER 2
#define PANE_WIDTH (PANEL_WIDTH * PANELS_NUMBER)
#define PANE_HEIGHT PANEL_HEIGHT
#define PIN_E 18

// placeholder for the matrix object
extern MatrixPanel_I2S_DMA *matrix;
extern VirtualMatrixPanel *dma_display;

extern volatile int brightness;

inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}
extern uint8_t back_ground[BCK_W][BCK_H][3];
extern uint16_t col_black;
extern uint16_t col_dark_grey;
extern uint16_t col_white;
extern uint16_t col_bright_white;

// extern uint8_t blue_grey[3];
extern uint8_t blue[3];

void display_init();
void print_back_ground();
void generate_blue_rectangles();
inline void drew_background_pixel(int x, int y);
void drewLine();
void drewLine(int y);

void display_init();
void display_startup();

void print_pixel(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b);

inline void drew_background_pixel(int x, int y) {
  dma_display->drawPixelRGB888(x, y, back_ground[x][y][0], back_ground[x][y][1],
                               back_ground[x][y][2]);
}
