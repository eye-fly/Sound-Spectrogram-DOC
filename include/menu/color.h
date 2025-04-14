#pragma once
#include <Arduino.h>

#include "menu/menu.h"

typedef struct rgb {
  uint8_t r, g, b;
} RGB;

inline RGB mix(RGB base_c, RGB mix_c, int ratio, int max_ratio) {
  int base_ratio = max_ratio - ratio;
  return {uint8_t((base_ratio * base_c.r + ratio * mix_c.r) /
                               max_ratio),
          static_cast<uint8_t>((base_ratio * base_c.g + ratio * mix_c.g) /
                               max_ratio),
          static_cast<uint8_t>((base_ratio * base_c.b + ratio * mix_c.b) /
                               max_ratio)};
}

extern RGB voice_C_col;
extern RGB mix_C_col;
extern RGB mix_flame_C_col;
void prindColourSample(RGB *col);

class ColorSelectMenu : public ListMenu {
  int hue, sat, light;
  RGB *rgb;

 public:
  ColorSelectMenu(String n, int h, int s, int l, RGB &rgb_col,
                  RGB *sampleBase = nullptr);

  // void print_content(int x, int y) override;

 private:
  void updateRGB();
  void updateRGBandPrint();
};