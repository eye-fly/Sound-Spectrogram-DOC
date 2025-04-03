#include <Arduino.h>

// Define number patterns as 5x3 grids
const int n_rows = 5;
const int n_cols = 3;
const char numbers[10][n_rows][n_cols] = {
    {{'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', ' ', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'}},  // 0
    {{' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'}},  // 1
    {{'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'},
     {'1', ' ', ' '},
     {'1', '1', '1'}},  // 2
    {{'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'}},  // 3
    {{'1', ' ', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'}},  // 4
    {{'1', '1', '1'},
     {'1', ' ', ' '},
     {'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'}},  // 5
    {{'1', '1', '1'},
     {'1', ' ', ' '},
     {'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'}},  // 6
    {{'1', '1', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'}},  // 7
    {{'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'}},  // 8
    {{'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'}}  // 9
};

void update_single_dig(int n, int x, int y, uint16_t col) {
  for (int i = 0; i < n_rows; i++) {
    for (int j = 0; j < n_cols; j++) {
      if (numbers[n][i][j] == '1') {
        dma_display->drawPixel(x + j, y + i, col);
      } else {
        drew_background_pixel(x + j, y + i);
      }
    }
  }
}

void erase_dig(int x, int y) {
  for (int i = 0; i < n_rows; i++) {
    for (int j = 0; j < n_cols; j++) {
      // if (numbers[1][i][j] == '1') {
      drew_background_pixel(x + j, y + i);
      // }
    }
  }
}
void update__small_num(uint n, int x, int y, uint16_t col) {
  int dig_p = 0;
  int diel = 10;
  int d;

  while (diel <= n) diel *= 10;
  diel /= 10;
  while (diel >= 1) {
    d = n / diel;
    d %= 10;
    update_single_dig(d, x, y, col);
    x += n_cols + 1;
    n %= diel;
    diel /= 10;
    dig_p++;
    // if (n < 10) break;
  }
  // if (dig_p == 0)
  //   update_single_dig(0, x, y, col);
  // else
  erase_dig(x, y);
}

const uint8_t arrow[7] = {
    0b00100,  // Row 0
    0b01110,  // Row 1
    0b11111,  // Row 2
    0b00100,  // Row 3
    0b00100,  // Row 4
    0b00100,  // Row 5
    0b00100   // Row 6
};
static void draw_arrow(int x, int y, uint16_t color, bool clear) {
  for (int row = 0; row < 7; row++) {
    for (int col = 4; col >= 0; col--) {  // 5 columns (bits 4 to 0)
      if (arrow[row] & (1 << col)) {      // Check if the bit is set
        if (clear) {
          drew_background_pixel(x - row, y + col);
        } else {
          dma_display->drawPixel(x - row, y + col, color);
        }
      }
    }
  }
}