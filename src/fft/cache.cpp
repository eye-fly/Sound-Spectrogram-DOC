#include "fft/cache.h"

#include "display.h"
#include "envVar.h"

int max_bucket;
float* s_sqew_table_psram = nullptr;

void initPSRAMTable() {
  if (s_sqew_table_psram == nullptr) {
    s_sqew_table_psram =
        (float*)heap_caps_malloc(PANE_WIDTH * sizeof(float), MALLOC_CAP_SPIRAM);
  }
  max_bucket = (SAMPLES / 2) * display_max_f / (DOWNSAMPLE_RATE / 2);

  if (s_sqew_table_psram != nullptr) {
    s_sqew_table_psram[0] = 0;
  }
  //   for (int x = 0; x < PANE_WIDTH; x++) {
  //     int fft_bucket = (max_bucket * x) / PANE_WIDTH;
  //     s_sqew_table_psram[x] =
  //         1.0f - (static_cast<float>(max_bucket * x) / PANE_WIDTH -
  //         fft_bucket);
  //   }
}

uint8_t CalAproxymateYValueFFTOutPreComp(int x, int channel, float boost) {
  const float sqew = s_sqew_table_psram[x];  // PSRAM access (cached)
  const int fft_bucket = max_bucket * x / PANE_WIDTH;

  //   float sqew = 1.0 - ((1.0 * max_bucket * x / PANE_WIDTH) - fft_bucket);
  float crr_y =
      (PANE_HEIGHT * (fftOut[fftOut_reading][channel][fft_bucket] * sqew));
  crr_y += (PANE_HEIGHT *
            (fftOut[fftOut_reading][channel][fft_bucket + 1] * (1.0 - sqew)));
  crr_y = (crr_y * boost) / 8.0f;

  uint8_t crr_y_int = crr_y;
  crr_y_int = max(0, crr_y_int - 1);
  crr_y_int = min(static_cast<uint8_t>(PANEL_HEIGHT - 1), crr_y_int);
  return crr_y_int;

  // Branchless clamping (faster than min/max on ESP32)
  //   uint8_t crr_y_int = static_cast<uint8_t>(crr_y);
  //   crr_y_int = (crr_y_int <= 1) ? 0 : (crr_y_int - 1);
  //   return (crr_y_int >= PANEL_HEIGHT - 1) ? PANEL_HEIGHT - 1 : crr_y_int;
}