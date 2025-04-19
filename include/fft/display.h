#pragma once

#include "menu/color.h"
// #include

class Context {
 public:
  int16_t x_start;
  int16_t y_start;
  int16_t y_mirror_start;
  const uint16_t x_len;
  std::function<u_int8_t(int)> y_f_mix;
  std::function<u_int8_t(int)> y_f_voc;

  u_int8_t* display_last_y_pos_mix;
  u_int8_t* display_last_y_pos_voc;

  Context(int16_t x_start, int16_t y_start, int16_t y_mirror_start,
          uint16_t x_len, std::function<uint8_t(int)> y_f_mix,
          std::function<uint8_t(int)> y_f_voc, uint8_t* display_last_y_pos_mix,
          uint8_t* display_last_y_pos_voc)
      : x_start(x_start),
        y_start(y_start),
        y_mirror_start(y_mirror_start),
        x_len(x_len),
        y_f_mix(y_f_mix),
        y_f_voc(y_f_voc),
        display_last_y_pos_mix(display_last_y_pos_mix),
        display_last_y_pos_voc(display_last_y_pos_voc) {}
};

RGB flameMix(RGB col, int16_t y, int16_t hDistance);
void update_fft_display(bool flip_y, Context* ctx);  // TODO add inline
