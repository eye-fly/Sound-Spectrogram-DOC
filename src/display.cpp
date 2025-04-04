#include "display.h"
volatile int brightness = 180;

uint8_t back_ground[BCK_W][BCK_H][3];
uint16_t col_black = color565(0, 0, 0);
uint16_t col_dark_grey = color565(65, 65, 65);
uint16_t col_white = color565(150, 150, 150);
uint16_t col_bright_white = color565(180, 180, 180);

uint8_t blue_grey[3] = {30, 35, 40};  // {30, 35, 40}
uint8_t blue[3] = {86, 162, 237};

MatrixPanel_I2S_DMA *matrix = nullptr;
VirtualMatrixPanel *dma_display = nullptr;

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
  mxconfig.setPixelColorDepthBits(6);

  matrix = new MatrixPanel_I2S_DMA(mxconfig);
  // let's adjust default brightness to about 75%

  // Allocate memory and start DMA display
  if (not matrix->begin())
    Serial.println("****** !KABOOM! I2S memory allocation failed ***********");

  matrix->setBrightness8(brightness);  // range is 0-255, 0 - 0%, 255 - 100%

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
  dma_display->fillScreenRGB888(140, 140, 140);
  delay(300);

  Serial.println("Fill screen: black");
  dma_display->fillScreenRGB888(0, 0, 0);
  generate_blue_rectangles();
  print_back_ground();

  drewLine(PANE_HEIGHT - 1);
  //   Serial.printf("refresh:%d bits:%d\n",
  //   dma_display->calculated_refresh_rate,
  // 0);
}

volatile int line_y_pos;
void drewLine(int y) {
  // clearLine()
  line_y_pos = y;
  for (int x = 0; x < PANE_WIDTH; x++) {
    // display_last_y_pos[x] = 0;
    dma_display->drawPixelRGB888(x, y, 140, 40, 10);
  }
}
void drewLine() {
  // clearLine()
  int y = line_y_pos;
  for (int x = 0; x < PANE_WIDTH; x++) {
    // display_last_y_pos[x] = 0;
    dma_display->drawPixelRGB888(x, y, 140, 40, 10);
  }
}
// void clearLine(int y) {
//   for (int x = 0; x < PANE_WIDTH; x++) {
//     // display_last_y_pos[x] = 0;
//     drew_background_pixel(x, y);
//   }
// }

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
