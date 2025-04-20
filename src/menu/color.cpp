#include "menu/color.h"

#include <Arduino.h>

#include "display.h"
#include "envVar.h"
#include "fft/display.h"
#include "util/col.h"
#include "util/util.h"

ColorSelectMenu::ColorSelectMenu(String n, int h, int s, int l, RGB &rgb_col,
                                 std::function<void()> updateFun)
    : hue(h),
      sat(s),
      light(l),
      rgb(&rgb_col),
      updateFun(updateFun),
      ListMenu(n, {}) {
  listCOntent.push_back(new MenuItem(
      "hue", &hue, 0, 255, 1, []() {},
      [this]() {
        updateRGBandPrint();
        this->updateFun();
      }));
  listCOntent.push_back(new MenuItem(
      "sat", &sat, 0, 255, 1, []() {},
      [this]() {
        updateRGBandPrint();
        this->updateFun();
      }));
  listCOntent.push_back(new MenuItem(
      "light", &light, 0, 255, 1, []() {},
      [this]() {
        updateRGBandPrint();
        this->updateFun();
      }));
  updateRGB();

  additionalDisplay = [this, &rgb_col]() {
    prindColourSample(&rgb_col);
    printParametersPrev();
  };
}

void prindColourSample(RGB *col) {
  const int max_x = 50;
  const int max_y = 40;

  static u_int8_t display_last_y_pos_mix[max_x];
  static u_int8_t display_last_y_pos_voc[max_x];
  static Context ctx = Context(
      5, 20, 20 + (max_y) / 2, max_x, [](int x) { return 0; },
      [](int x) { return 0; }, display_last_y_pos_mix, display_last_y_pos_voc);

  ctx.y_f_mix = [](int x) {
    return 1.0 * max_y * 8 * 8 / PANE_HEIGHT *
           normalPDF((1.0 * x / (max_x / 6)) - 1);
  };

  ctx.y_f_mix = [](int x) {
    u_int8_t y = 1.0 * max_y * 8 * 8 / PANE_HEIGHT *
                 centerPDF(x, max_x * 2 / 5, normalPDF);

    u_int8_t y2 =
        1.0 * max_y * 8 * 3 / PANE_HEIGHT * centerPDF(x, max_x, flatPDF);
    return y > y2 ? y : y2;
  };

  ;
  update_fft_display(false, &ctx);
}

void ColorSelectMenu::updateRGB() {
  (*rgb) = hsl2rgb(1.0 * hue / 255, 1.0 * sat / 255, 1.0 * light / 255);
}

void ColorSelectMenu::printParametersPrev() {
  printParameterPrev(&hue, 1);
  printParameterPrev(&sat, 1 + 8);
  printParameterPrev(&light, 1 + 8 + 8);
}

void ColorSelectMenu::printParameterPrev(int *parameter, int16_t disp_y) {
  RGB col_prev;

  const int par_val = (*parameter);
  const int16_t disp_x = 200;
  const int len = 20;

  int16_t crr_line = par_val * len / (255);
  for (int x = 0; len > x; x++) {
    (*parameter) = 255 * x / len;
    col_prev = hsl2rgb(1.0 * hue / 255, 1.0 * sat / 255, 1.0 * light / 255);
    if (x == crr_line) {
      draw_line_v(disp_x + x, disp_y, 7, col_prev);
    } else {
      drew_background_pixel(disp_x + x, disp_y);
      drew_background_pixel(disp_x + x, disp_y + 6);
      draw_line_v(disp_x + x, disp_y + 1, 5, col_prev);
    }
  }

  (*parameter) = par_val;
}
void ColorSelectMenu::updateRGBandPrint() {
  ColorSelectMenu::updateRGB();
  // update__small_num(8, 120, 25, col_white);
  prindColourSample(rgb);
  printParametersPrev();
  update__small_num(rgb->r, 120, 25, col_white);
  update__small_num(rgb->g, 120, 30, col_white);
  update__small_num(rgb->b, 120, 35, col_white);

  // printParameterPrev(&hue, 1);
}

// void ColorSelectMenu::print_content(int x, int y) {
//   ListMenu::print_content(x, y);
//   ColorSelectMenu::prindColourSample(*rgb);
// }