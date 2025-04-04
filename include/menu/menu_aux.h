#pragma once

#include <Arduino.h>

// Define number patterns as 5x3 grids
constexpr int n_rows = 5;
constexpr int n_cols = 3;
extern const char numbers[10][n_rows][n_cols];

void update_single_dig(int n, int x, int y, uint16_t col);

void erase_dig(int x, int y);
void update__small_num(uint n, int x, int y, uint16_t col);

extern const uint8_t arrow[7];

void draw_arrow(int x, int y, uint16_t color, bool clear);

int textWidth(String s);

void text(String text, int x, int y, uint16_t col, uint16_t bac_col);