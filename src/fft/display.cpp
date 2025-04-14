#include "fft/display.h"

#include "envVar.h"

RGB flameMix(RGB col, int y, int hDistance) {
  //   toYellow = min(f_left[y], f_right[x][y]) - 3;  // f_right[x][y]
  int toYellow = min(hDistance - flame_gradd_offset, (2 * y) - 3);
  toYellow = max(toYellow, 0);
  toYellow = min(toYellow, int(flame_gradient_len));

  return mix(col, mix_flame_C_col, toYellow, flame_gradient_len);
}
