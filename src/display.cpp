#include "display.h"

#include "menu/color.h"
#include "menu/menu_aux.h"
volatile int brightness = 180;

uint8_t back_ground[BCK_W][BCK_H][3];
uint16_t col_black = color565(0, 0, 0);
uint16_t col_dark_grey = color565(65, 65, 65);
uint16_t col_white = color565(150, 150, 150);
uint16_t col_bright_white = color565(180, 180, 180);

uint8_t blue_grey[3] = {7, 25, 46};  // {30, 35, 40}
uint8_t blue[3] = {86, 162, 237};

MatrixPanel_I2S_DMA *matrix = nullptr;
VirtualMatrixPanel *dma_display = nullptr;

void gen_brightness_reorder();

void display_init() {
  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_width = PANEL_WIDTH;
  mxconfig.mx_height = PANEL_HEIGHT;      // we have 64 pix heigh panels
  mxconfig.chain_length = PANELS_NUMBER;  // we have 2 panels chaineds~
  mxconfig.gpio.e = PIN_E;
  mxconfig.gpio.b1 = R1_PIN_DEFAULT;
  mxconfig.gpio.b2 = R2_PIN_DEFAULT;
  mxconfig.gpio.r1 = G1_PIN_DEFAULT;
  mxconfig.gpio.r2 = G2_PIN_DEFAULT;
  mxconfig.gpio.g1 = B1_PIN_DEFAULT;
  mxconfig.gpio.g2 = B2_PIN_DEFAULT;
  mxconfig.clkphase = false;
  mxconfig.setPixelColorDepthBits(8);
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_16M;

  matrix = new MatrixPanel_I2S_DMA(mxconfig);

  Serial.printf("Actual I2S clock: %d Hz\n", mxconfig.i2sspeed);

  // Allocate memory and start DMA display
  if (not matrix->begin())
    Serial.println("****** !KABOOM! I2S memory allocation failed ***********");

  matrix->setBrightness8(brightness);  // range is 0-255, 0 - 0%, 255 - 100%

  Serial.printf("calculated_refresh_rate: %d Hz\n",
                matrix->calculated_refresh_rate);

  dma_display = new VirtualMatrixPanel((*matrix), 1, 2, PANEL_WIDTH,
                                       PANEL_HEIGHT, CHAIN_TOP_LEFT_DOWN);

  dma_display->fillScreen(dma_display->color444(0, 0, 0));
  dma_display->drawDisplayTest();

  delay(400);
  // well, hope we are OK, let's draw some colors first :)
  Serial.println("Fill screen: RED");
  dma_display->fillScreenRGB888(160, 0, 0);
  delay(300);

  Serial.println("Fill screen: GREEN");
  dma_display->fillScreenRGB888(0, 160, 0);
  delay(300);

  Serial.println("Fill screen: BLUE");
  dma_display->fillScreenRGB888(0, 0, 160);
  delay(300);

  Serial.println("Fill screen: Neutral White");
  dma_display->fillScreenRGB888(144, 144, 144);
  delay(300);

  Serial.println("Fill screen: black");
  dma_display->fillScreenRGB888(0, 0, 0);
  generate_blue_rectangles();
  print_back_ground();

  drewLine(PANE_HEIGHT - 1);
  //   Serial.printf("refresh:%d bits:%d\n",
  //   dma_display->calculated_refresh_rate,
  // 0);

  gen_brightness_reorder();
}

volatile int line_y_pos;
void drewLine(int y) {
  // clearLine()
  line_y_pos = y;
  for (int x = 0; x < PANE_WIDTH; x++) {
    // display_last_y_pos[x] = 0;
    dma_display->drawPixelRGB888(x, y, mix_C_col.r, mix_C_col.g, mix_C_col.b);
  }
}
void drewLine() {
  // clearLine()
  int y = line_y_pos;
  drewLine(line_y_pos);
  // for (int x = 0; x < PANE_WIDTH; x++) {
  //   // display_last_y_pos[x] = 0;
  //   dma_display->drawPixelRGB888(x, y, col_mix.r, 40, 10);
  // }
}
// void clearLine(int y) {
//   for (int x = 0; x < PANE_WIDTH; x++) {
//     // display_last_y_pos[x] = 0;
//     drew_background_pixel(x, y);
//   }
// }

// fix binary-code modulation (rgb(193,0,0) is brighter than rgb(194,0,0)) also
// 145 brighter that 146
uint8_t brightness_reorder[255];
void gen_brightness_reorder() {
  int coll_offset_1l = 18;  // 19
  int coll_offset_1h = 13;  // 16

  int coll_offset_2l = 25;
  int coll_offset_2h = 24;

  int coll_offset = 22;
  int pivot1 = 145;  // the wrong difference is bettween 145 and 146
  int pivot2 = 193;  // the wrong difference is bettween 193 and 194
  for (int i = 0; 256 > i; i++) {
    if ((i < pivot1 && i > pivot1 - coll_offset_1l))
      brightness_reorder[i] = i + coll_offset_1l;
    else if (i > pivot1 && i < pivot1 + coll_offset_1h)
      brightness_reorder[i] = i - coll_offset_1h;

    else if ((i > pivot2 && i < pivot2 + coll_offset_2h)) {
      brightness_reorder[i] = i - coll_offset_2h;
    } else if ((i < pivot2 && i > pivot2 - coll_offset_2l)) {
      brightness_reorder[i] = i + coll_offset_2l;
    } else
      brightness_reorder[i] = i;
  }
}

void print_pixel(int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b) {
  r = brightness_reorder[r];
  g = brightness_reorder[g];
  b = brightness_reorder[b];

  update__small_num((r > 0 ? r : 0), 120, 45, col_white);
  dma_display->drawPixelRGB888(x, y, r, g, b);
}

void print_back_ground() {
  for (int y = 0; PANE_HEIGHT > y; y++) {
    if (y == line_y_pos) continue;
    for (int x = 0; PANE_WIDTH > x; x++) {
      dma_display->drawPixelRGB888(x, y, back_ground[x][y][0],
                                   back_ground[x][y][1], back_ground[x][y][2]);
    }
  }
}

void fill_square(int x, int y, int size, uint8_t c[3]) {
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      back_ground[x + i][y + j][0] = c[0];  // Red component
      back_ground[x + i][y + j][1] = c[1];  // Green component
      back_ground[x + i][y + j][2] = c[2];  // Blue component
    }
  }
}
void generate_blue_rectangles() {
  // form down grey ones
  // randomSeed(analogRead(0)); // Use noise from an unconnected analog pin
  for (int i = 1; (i + 1) < BCK_W; i += 3) {
    int endY = random(max(BCK_H / 2 - HALFPOINT_WIGGLE, 0),
                      min(BCK_H / 2 + HALFPOINT_WIGGLE, BCK_H - 1));
    int y = 1;
    while (y + 1 < endY) {
      if (random(0, 100) > 75) {
        fill_square(i, y, 2, blue_grey);
      }
      y += 2;
    }
  }
}
