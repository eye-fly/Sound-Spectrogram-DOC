#pragma once
#include <Arduino.h>

typedef struct rgb {
  uint8_t r, g, b;
} RGB;

RGB hsl2rgb(float h, float s, float l);

inline RGB mix(RGB base_c, RGB mix_c, int16_t ratio, int16_t max_ratio) {
  int16_t base_ratio = max_ratio - ratio;
  return {static_cast<uint8_t>((base_ratio * base_c.r + ratio * mix_c.r) /
                               max_ratio),
          static_cast<uint8_t>((base_ratio * base_c.g + ratio * mix_c.g) /
                               max_ratio),
          static_cast<uint8_t>((base_ratio * base_c.b + ratio * mix_c.b) /
                               max_ratio)};
}

extern RGB voice_C_col;
extern RGB mix_C_col;
extern RGB mix_flame_C_col;

extern RGB rectangles_col;
extern RGB blue_grey;