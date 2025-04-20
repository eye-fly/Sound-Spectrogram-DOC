#pragma once
#include <Arduino.h>

#include "menu/menu.h"
#include "util/col.h"

void prindColourSample(RGB *col);

class ColorSelectMenu : public ListMenu {
  int hue, sat, light;
  RGB *rgb;
  std::function<void()> updateFun;

 public:
  ColorSelectMenu(
      String n, int h, int s, int l, RGB &rgb_col,
      std::function<void()> updateFun = []() {});

  // void print_content(int x, int y) override;

 private:
  void updateRGB();
  void updateRGBandPrint();
  void printParameterPrev(int *parameter, int16_t disp_y);
  void printParametersPrev();
};