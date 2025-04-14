#include "menu/color.h"

#include <Arduino.h>

#include "display.h"
#include "envVar.h"
#include "fft/display.h"
#include "util.h"

RGB voice_C_col = {27, 175, 89};
RGB mix_C_col = {190, 70, 33};
RGB mix_flame_C_col = {96, 166, 33};

// typedef struct hsl {
//   float h, s, l;
// } HSL;

// /*
//  * Converts an RGB color value to HSL. Conversion formula
//  * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
//  * Assumes r, g, and b are contained in the set [0, 255] and
//  * returns HSL in the set [0, 1].
//  */
// HSL rgb2hsl(float r, float g, float b) {
//   HSL result;

//   r /= 255;
//   g /= 255;
//   b /= 255;

//   float mx = max(max(r, g), b);
//   float mn = min(min(r, g), b);

//   result.h = result.s = result.l = (mx + mn) / 2;

//   if (mx == mn) {
//     result.h = result.s = 0;  // achromatic
//   } else {
//     float d = mx - mn;
//     result.s = (result.l > 0.5) ? d / (2 - mx - mn) : d / (mx + mn);

//     if (mx == r) {
//       result.h = (g - b) / d + (g < b ? 6 : 0);
//     } else if (mx == g) {
//       result.h = (b - r) / d + 2;
//     } else if (mx == b) {
//       result.h = (r - g) / d + 4;
//     }

//     result.h /= 6;
//   }

//   return result;
// }

////////////////////////////////////////////////////////////////////////

/*
 * Converts an HUE to r, g or b.
 * returns float in the set [0, 1].
 */
float hue2rgb_aux(float p, float q, float t) {
  if (t < 0) t += 1;
  if (t > 1) t -= 1;
  if (t < 1. / 6) return p + (q - p) * 6 * t;
  if (t < 1. / 2) return q;
  if (t < 2. / 3) return p + (q - p) * (2. / 3 - t) * 6;

  return p;
}

float hue2rgb(float p, float q, float t) {
  float am = hue2rgb_aux(p, q, t);
  if (am < 0)
    return 0;
  else
    return am;
}

/*
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns RGB in the set [0, 255].
 */
RGB hsl2rgb(float h, float s, float l) {
  RGB result;

  if (0 == s) {
    result.r = result.g = result.b = l;  // achromatic
  } else {
    float q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    float p = 2 * l - q;
    result.r = hue2rgb(p, q, h + 1. / 3) * 255;
    // result.r
    result.g = hue2rgb(p, q, h) * 255;
    result.b = hue2rgb(p, q, h - 1. / 3) * 255;
  }

  return result;
}

ColorSelectMenu::ColorSelectMenu(String n, int h, int s, int l, RGB &rgb_col,
                                 RGB *sampleBase)
    : hue(h),
      sat(s),
      light(l),
      rgb(&rgb_col),
      ListMenu(n, {}, [&rgb_col, sampleBase]() {
        prindColourSample(sampleBase ? sampleBase : &rgb_col);
      }) {
  listCOntent.push_back(
      new MenuItem("hue", &hue, 0, 255, 1, [this]() { updateRGBandPrint(); }));
  listCOntent.push_back(
      new MenuItem("sat", &sat, 0, 255, 1, [this]() { updateRGBandPrint(); }));
  listCOntent.push_back(new MenuItem("light", &light, 0, 255, 1,
                                     [this]() { updateRGBandPrint(); }));
  updateRGB();
}

void prindColourSample(RGB *col) {
  const int max_x = 40;
  const int max_y = 34;

  int start_x = 0;
  int start_y = max_y + 8;

  unsigned short last_d_d[max_y];
  for (int y = 0; y < max_y; y++) {
    last_d_d[y] = 0;
  }

  for (int x = 0; x < max_x / 2; x++) {
    int y = max_y * normalPDF((1.0 * x / (max_x / 2)) - 1);

    for (int crr_y = 0; y > crr_y; crr_y++) {
      // int toYellow = min(
      //     int(last_d_d[crr_y]),
      //     (2 * crr_y) -
      //         3);  // TODO: implement and use here actual toyellow
      //              // method so the same code isn;t copied in 2 different
      //              places
      // toYellow = max(toYellow, 0);
      // toYellow = min(toYellow, int(flame_gradient_len));

      // = mix(*col, mix_flame_C_col, toYellow, flame_gradient_len);

      // RGB crr_col = flameMix(*col, crr_y, last_d_d[crr_y]);
      RGB crr_col = flameMix(*col, crr_y, 0);

      print_pixel(start_x + x, start_y - crr_y, crr_col.r, crr_col.g,
                  crr_col.b);
      print_pixel(max_x - x, start_y - crr_y, crr_col.r, crr_col.g, crr_col.b);
      last_d_d[crr_y] += 1;
    }
  }
  for (int crr_y = 0; max_y > crr_y; crr_y++) {
    RGB crr_col = flameMix(*col, crr_y, last_d_d[crr_y]);

    print_pixel(start_x + (max_x / 2), start_y - crr_y, crr_col.r, crr_col.g,
                crr_col.b);
    print_pixel(max_x - (max_x / 2), start_y - crr_y, crr_col.r, crr_col.g,
                crr_col.b);
  }
}

void ColorSelectMenu::updateRGB() {
  (*rgb) = hsl2rgb(1.0 * hue / 255, 1.0 * sat / 255, 1.0 * light / 255);
}

void ColorSelectMenu::updateRGBandPrint() {
  ColorSelectMenu::updateRGB();
  // update__small_num(8, 120, 25, col_white);
  prindColourSample(rgb);
  update__small_num(rgb->r, 120, 25, col_white);
  update__small_num(rgb->g, 120, 30, col_white);
  update__small_num(rgb->b, 120, 35, col_white);
}

// void ColorSelectMenu::print_content(int x, int y) {
//   ListMenu::print_content(x, y);
//   ColorSelectMenu::prindColourSample(*rgb);
// }