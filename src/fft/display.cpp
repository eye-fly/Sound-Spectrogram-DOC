#include "fft/display.h"

#include "envVar.h"

RGB flameMix(RGB col, int16_t y, int16_t hDistance) {
  int16_t flame_grand_l = static_cast<int16_t>(flame_gradient_len);
  int16_t toYellow = min(hDistance - static_cast<int16_t>(flame_gradd_offset),
                         (int16_t(2) * y) - int16_t(3));
  toYellow = max(toYellow, int16_t(0));
  toYellow = min(toYellow, flame_grand_l);

  return mix(col, mix_flame_C_col, toYellow, flame_grand_l);
}

class Effects {
 public:
  int16_t f_left[PANEL_HEIGHT];
  int16_t f_right[PANE_WIDTH][PANE_HEIGHT];
};

Effects effects;

inline void UpdateDisplay_deactivate_channel(bool mirror, Context* ctx, int x,
                                             int crr_y,
                                             uint8_t* display_last_y_pos) {
  if (mirror) {
    const int y_start = PANE_HEIGHT - 1 - ctx->y_mirror_start;
    for (int y = display_last_y_pos[x]; y > crr_y; y--) {
      drew_background_pixel(x, y_start + y);
      drew_background_pixel(x, y_start - y);
    }
  } else {
    const int y_start = PANE_HEIGHT - 1 - ctx->y_start;
    for (int y = display_last_y_pos[x]; y > crr_y; y--) {
      drew_background_pixel(x, y_start - y);
    }
  }
  display_last_y_pos[x] = crr_y;
}

inline void UpdateDisplay_activate_channel(bool mirror, Context* ctx,
                                           int minimal_y, int x, int crr_y,
                                           RGB col) {
  RGB col_mix = col;

  const int16_t display_x = x + ctx->x_start;

  if (mirror) {
    const int display_y = PANE_HEIGHT - 1 - ctx->y_mirror_start;

    for (int16_t y = minimal_y; y <= crr_y; y++) {
      if (falame_colour_enable) {
        effects.f_left[y] += static_cast<int16_t>(1);

        col_mix =
            flameMix(col, y, min(effects.f_left[y], effects.f_right[x][y]));
      }

      // if (flip_y) {
      //   display_y = ctx->y_start + y;
      //   print_pixel(display_x, display_y, col_mix.r, col_mix.g, col_mix.b);
      //   // display_y_flip++;
      // } else {
      print_pixel(display_x, display_y - y, col_mix.r, col_mix.g, col_mix.b);
      print_pixel(display_x, display_y + y, col_mix.r, col_mix.g, col_mix.b);
      // }
    }
  } else {
    const int16_t display_y = PANEL_HEIGHT - 1 - ctx->y_start;

    for (int16_t y = minimal_y; y <= crr_y; y++) {
      if (falame_colour_enable) {
        effects.f_left[y] += static_cast<int16_t>(1);

        col_mix =
            flameMix(col, y, min(effects.f_left[y], effects.f_right[x][y]));
      }

      // display_y = PANEL_HEIGHT - 1 - ctx->y_start - y;
      print_pixel(display_x, display_y - y, col_mix.r, col_mix.g, col_mix.b);
    }
  }

  if (falame_colour_enable) {
    for (int y = crr_y + 1; PANEL_HEIGHT > y; y++) {
      effects.f_left[y] = 0;
    }
  }
}

inline void CalkDym(Context* ctx, std::function<u_int8_t(int)> y_f) {
  // Left
  for (uint16_t y = 1; y < PANEL_HEIGHT; y++) {
    effects.f_left[y] = 0;
    effects.f_right[ctx->x_len - 1][y] = 0;
  }
  // right
  int crr_y;
  for (int16_t i = ctx->x_len - 2; i >= 0; i--) {
    crr_y = y_f(i);

    for (int y = 1; y <= crr_y; y++) {
      if (i < ctx->x_len - 1) {
        effects.f_right[i][y] = effects.f_right[i + 1][y] + 1;
      }
    }

    for (int y = crr_y + 1; y < PANEL_HEIGHT; y++) {
      if (i > 0) {
        if (effects.f_right[i][y] == 0) break;
        effects.f_right[i][y] = 0;
      }
    }
  }
}

void update_fft_display(bool flip_y, Context* ctx) {
  int crr_y_mix;
  int crr_y_voc = 0;

  CalkDym(ctx, ctx->y_f_mix);
  for (int x = 0; x < ctx->x_len; x++) {
    crr_y_mix = ctx->y_f_mix(x);

    if (enable_voc_channel) {
      crr_y_voc = ctx->y_f_voc(x);
    }

    UpdateDisplay_deactivate_channel(flip_y, ctx, x, max(crr_y_mix, crr_y_voc),
                                     ctx->display_last_y_pos_mix);

    if (crr_y_voc == 0 && ctx->display_last_y_pos_voc[x] > 0) {
      print_pixel(x, PANE_HEIGHT - 1, (mix_C_col.r), (mix_C_col.g),
                  (mix_C_col.b));  /// TODO add proper
    }

    ctx->display_last_y_pos_voc[x] = crr_y_voc;

    UpdateDisplay_activate_channel(flip_y, ctx, crr_y_voc + 1, x, crr_y_mix,
                                   mix_C_col);
  }

  if (enable_voc_channel) {
    CalkDym(ctx, ctx->y_f_voc);
    for (int x = 0; x < ctx->x_len; x++) {
      crr_y_voc = ctx->y_f_voc(x);

      if (crr_y_voc > 0) {
        print_pixel(x, PANE_HEIGHT - 1, voice_C_col.r, voice_C_col.g,
                    voice_C_col.b);  /// TODO add proper method
                                     /// so mirror works properly
      }
      UpdateDisplay_activate_channel(flip_y, ctx, 1, x, crr_y_voc, voice_C_col);
    }
  }
}