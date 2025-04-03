#include <Arduino.h>
#include <driver/i2s.h>
#include <driver/ledc.h>

// #include <arduinoFFT.h>
#include "fft.h"
// #include <esp_dsp.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include "display.h"
#include "esp_heap_caps.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "menu.h"
#include "util.hpp"
///
#include "color.h"

// i2s audio in
#define I2S_NUM I2S_NUM_0
#define I2S_BCK_IO 2   // Bit Clock (BCK) pin on ESP32
#define I2S_WS_IO 32   // Word Select (LRCK) pin on ESP32
#define I2S_DO_IO 33   // Data Output (DOUT) pin from PCM1808
#define I2S_MCLK_IO 0  // SCKI system clock output pin

#define MULT 2
#define CHANNEL_NUMBER 2
#define SAMPLE_RATE 10000  // 1 kHz sample rate
#define DOWNSAMPLE_RATE \
  (350 * MULT * 2)  // 625    // Target sample rate (downsampled)
#define FFT_MAX_F (350 * MULT)
#define DOWNSAMPLE_FACTOR \
  (SAMPLE_RATE / DOWNSAMPLE_RATE)  // Ratio for downsampling (5)
#define BUFFER_SIZE \
  (100)  //*10                                // Buffer size for right channel
         // data
#define FILTER_ORDER 2  // Order of the low-pass filter (adjustable)

#define DMA_BUF_LEN \
  BUFFER_SIZE  // TODO delete                     // DMA buffer length (8 * 16)

// FFT
#define SAMPLES \
  (128 * MULT * 2)  //(256) // Number of samples (must be a power of 2)
// #define ZERO_PADDING (145 + 256)  //(128+256)
// #define WINNDOWING_RATIO 1.0
// #define DCOFFSET 30420.0
#define SCALEDOWN ((2 << 20) * MULT)

// Create a handle for the mutex
SemaphoreHandle_t mutex;
TickType_t xBlockTime = pdMS_TO_TICKS(1);

volatile int non_zero_samples = 217;  // 111
#define DEFAULT_VOLUME_ADJUSTMENT 50
volatile int volue_adjustment = 25;
volatile int use_log_scale = 3;
volatile int display_max_f = 650;  // FFT_MAX_F;

volatile int falame_colour_enable = 1;
volatile int flame_gradient_len = 8;
volatile int miror = 0;

volatile int enable_voc_channel = 0;
// unsigned int zero_padding = SAMPLES - non_zero_samples;

// double vReal[SAMPLES]; // Array for storing real part of the signal
// double vImag[SAMPLES];
float fftOut[3][CHANNEL_NUMBER][SAMPLES / 2];
int fftOut_reading = -1;
int fftOut_avaiable = 0;

int32_t i2sBuffer[DMA_BUF_LEN * 2];

int16_t serial_i = 0;

/* Create FFT object */
// ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES,
// DOWNSAMPLE_RATE); Create fft plan and let it allocate arrays
fft_config_t* fft_analysis =
    fft_init(SAMPLES, FFT_REAL, FFT_FORWARD, NULL, NULL);

RGB voice_C_col = {27, 175, 89};
RGB mix_C_col = {190, 70, 33};
RGB mix_flame_C_col = {96, 166, 33};
// TODO: add dim line colur so its acts dimnamicly (but check how it affect
// preformance)

void menu_setup() {
  mainMenu.listCOntent.push_back(createSavePopup(&mainMenu));

  soundMenu.listCOntent.push_back(
      new MenuItem("FFT non0 samp", &non_zero_samples, 1,
                   []() { preprocess_windowing(non_zero_samples); }));
  soundMenu.listCOntent.push_back(
      new MenuItem("volume anj", &volue_adjustment));
  soundMenu.listCOntent.push_back(
      new MenuItem("use log scale", &use_log_scale));
  soundMenu.listCOntent.push_back(new MenuItem("max f", &display_max_f));
  soundMenu.listCOntent.push_back(
      new MenuItem("en voc ch", &enable_voc_channel));

  displayMenu.listCOntent.push_back(new ListMenu(
      "flame eff opt",
      {new ColorSelectMenu("center col", 55, 170, 80, mix_flame_C_col),
       new MenuItem("en eff", &falame_colour_enable),
       new MenuItem("eff grad", &flame_gradient_len)}));

  displayMenu.listCOntent.push_back(
      new MenuItem("brightness", &brightness, 8,
                   []() { matrix->setBrightness8(brightness); }));
  displayMenu.listCOntent.push_back(new MenuItem("mirror", &miror, 1, []() {
    if (miror) {
      drewLine(PANE_HEIGHT / 2);
    } else {
      drewLine(PANE_HEIGHT - 1);
    }
    print_back_ground();
  }));
  displayMenu.listCOntent.push_back(
      new ColorSelectMenu("voc col", 99, 175, 89, voice_C_col));
  displayMenu.listCOntent.push_back(
      new ColorSelectMenu("mix col", 6, 178, 112, mix_C_col));  // 6
}

int serial_period = 0;
inline bool UpdateDisplayVar(bool, int, int);
inline void UpdateDisplayHalfRes();
void audio_master();

void core1_tast(void* pvParameters) {
  esp_task_wdt_delete(NULL);
  int n_times = 140;
  int times_i = 0;
  int times_sum = 0;
  int times_max = 0;
  while (1) {
    unsigned long startTime = millis();

    show_update_time();

    bool new_frame;
    if (miror) {
      new_frame = UpdateDisplayVar(true, 32, PANEL_HEIGHT / 2);
    } else {
      new_frame = UpdateDisplayVar(false, 0, PANEL_HEIGHT);
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);

    if (new_frame) {
      update_menu();  // TODO: Might consider to Update menu even when we don't
                      // update fft diplay

      int p_time = millis() - startTime;
      times_sum += p_time;
      times_max = max(times_max, p_time);
      times_i++;
      if (times_i == n_times) {
        display_update_avg_ms = times_sum / n_times;
        display_update_max_ms = times_max;
        times_sum = 0;
        times_max = 0;
        times_i = 0;
      }
    }
    // Serial.print(">D: ");
    // Serial.println(processingTime);
  }
}

void setup() {
  Serial.begin(921600);  // Start serial communication for debugging
  mutex = xSemaphoreCreateMutex();

  // Serial.print("cpu f:");
  //  Serial.println(ets_get_cpu_frequency());
  // delay(10000);
  // // disableWireless();
  // Serial.println("I2S driver installed successfully2");

  // size_t free_dma_memory = heap_caps_get_free_size(MALLOC_CAP_DMA);
  // Serial.print("Free DMA-capable memory: ");
  // Serial.println(free_dma_memory);
  // // esp_intr_dump();
  display_init();
  menu_init();
  menu_setup();  // add itemes that change glabal variables

  xTaskCreatePinnedToCore(core1_tast, "i2UpdateDisplayVars_task", 2000, NULL, 1,
                          NULL, 0);

  // esp_intr_dump();
  // esp_timer_dump(stdout);
  // Serial.print("Free DMA-capable memory: ");
  // Serial.println(free_dma_memory);
  // // i2s_init(); // Initialize I2S to generate clocks

  audio_master();
}

void loop() {}

// channel_nr is 0 or 1 as samples are interchaning in i2sbuffer
void down_sample(int channel_nr, int* downsampleCounter,
                 double* downsampleValue, int32_t* filterBuffer,
                 double* downsampleBuffer, int* downsampleBuffer_i) {
  for (int i = 0; i < DMA_BUF_LEN * 2; i += 2) {
    // Shift filter buffer and add new sample
    for (int j = FILTER_ORDER - 1; j > 0; j--) {
      filterBuffer[j] = filterBuffer[j - 1];
    }
    filterBuffer[0] = (i2sBuffer[i + channel_nr]);

    // Compute the moving average (simple low-pass filter)
    int32_t filteredValue = 0;
    for (int j = 0; j < FILTER_ORDER; j++) {
      filteredValue += filterBuffer[j];
    }
    filteredValue /= FILTER_ORDER;

    (*downsampleCounter)++;
    (*downsampleValue) += filteredValue;

    if ((*downsampleCounter) >= DOWNSAMPLE_FACTOR) {
      downsampleBuffer[(*downsampleBuffer_i)] =
          (*downsampleValue) / (DOWNSAMPLE_FACTOR);

      (*downsampleBuffer_i)++;

      // serial_i++;
      // serial_i %= 2000;
      // Serial.print(">c:");
      // Serial.print(serial_i);
      // Serial.print(":");
      // Serial.print((int16_t)(downsampleValue)); // Output downsampled
      // value Serial.println("|xy");

      (*downsampleCounter) = 0;
      (*downsampleValue) = 0;
    }
  }
}

void fft(float* fftIn, double* downsampleBuffer, int downsampleBuffer_i) {
  // live volume / y axis scaling
  unsigned long long scaledown = (SCALEDOWN)*non_zero_samples;
  scaledown *= volue_adjustment;
  scaledown /= DEFAULT_VOLUME_ADJUSTMENT;

  // Shift older data
  for (int i = SAMPLES - 1; i >= downsampleBuffer_i; i--) {
    fftIn[i] = fftIn[i - downsampleBuffer_i];
  }
  for (int i = downsampleBuffer_i - 1; i >= 0; i--) {
    fftIn[i] = downsampleBuffer[downsampleBuffer_i - i - 1];
    fftIn[i] /= scaledown;
  }
  // Zero padding
  unsigned int zero_padding = SAMPLES - non_zero_samples;
  for (int i = SAMPLES - 1; i > SAMPLES - 1 - zero_padding; i--) {
    fftIn[i] = 0;
  }

  for (int i = 0; i < SAMPLES; i++) {
    fft_analysis->input[i] = fftIn[i];
  }
  // Apply Windowing fun
  for (int i = 0; i < SAMPLES - zero_padding; i++) {
    fft_analysis->input[i] *= windowValues[i];
  }
  fft_execute(fft_analysis);
}

void single_channel_process_aux(int channel_nr, int fft_array_i,
                                int fft_chanel_out, int32_t* filterBuffer,
                                int* downsampleCounter, double* downsampleValue,
                                double* downsampleBuffer, float* fftIn) {
  // Serial.println("test");

  int32_t downsampleBuffer_i = 0;
  down_sample(channel_nr, downsampleCounter, downsampleValue, filterBuffer,
              downsampleBuffer, &downsampleBuffer_i);

  fft(fftIn, downsampleBuffer, downsampleBuffer_i);

  int nr = fft_array_i;
  for (int i = 0; i < SAMPLES / 2; i++) {
    fftOut[nr][fft_chanel_out][i] =
        pow(fft_analysis->output[(i + 1) * 2], 2) +
        pow(fft_analysis->output[(i + 1) * 2 + 1], 2);
    fftOut[nr][fft_chanel_out][i] = sqrt(fftOut[nr][fft_chanel_out][i]);

    if (use_log_scale == 1) {
      // apparently fftout max is 8 -> then to the top of scrren
      // b^4 / 4
      // fftOut[nr][i] = log(1 + fftOut[nr][i] * fftOut[nr][i]) * 1.5;
      // fftOut[nr][i] = log(0.15 * fftOut[nr][i] + 1) * 8.5;

      fftOut[nr][fft_chanel_out][i] =
          log(0.25 * fftOut[nr][fft_chanel_out][i] + 1) * 5.5;
    }
    if (use_log_scale == 2) {
      fftOut[nr][fft_chanel_out][i] =
          log(0.4 * fftOut[nr][fft_chanel_out][i] + 1) * 3.7;
    }
    if (use_log_scale == 3) {
      fftOut[nr][fft_chanel_out][i] =
          log(0.65 * fftOut[nr][fft_chanel_out][i] + 1) * 2.6;
    }
  }
}

void single_channel_process_mix(int fft_array_i) {
  static int32_t filterBuffer[FILTER_ORDER];
  static int downsampleCounter = 0;
  static double downsampleValue = 0;
  static double downsampleBuffer[(DMA_BUF_LEN / DOWNSAMPLE_FACTOR) + 1];

  static float fftIn[SAMPLES];
  single_channel_process_aux(0, fft_array_i, 0, filterBuffer,
                             &downsampleCounter, &downsampleValue,
                             downsampleBuffer, fftIn);
}
void single_channel_process_voc(int fft_array_i) {
  static int32_t filterBuffer[FILTER_ORDER];
  static int downsampleCounter = 0;
  static double downsampleValue = 0;
  static double downsampleBuffer[(DMA_BUF_LEN / DOWNSAMPLE_FACTOR) + 1];

  static float fftIn[SAMPLES];
  single_channel_process_aux(1, fft_array_i, 1, filterBuffer,
                             &downsampleCounter, &downsampleValue,
                             downsampleBuffer, fftIn);
}

void audio_process() {
  size_t bytesRead;

  while (1) {
    // Read from ADC using I2S DMA
    esp_err_t result =
        i2s_read(I2S_NUM, &i2sBuffer, DMA_BUF_LEN * sizeof(int32_t) * 2,
                 &bytesRead, portMAX_DELAY);

    if (result == ESP_OK && bytesRead > 0) {
      // Record start time
      unsigned long startTime = millis();

      // Downsample from 16kHz to 1kHz with low-pass filtering
      if (bytesRead != DMA_BUF_LEN * sizeof(int32_t) * 2) {
        Serial.print(">read wierd num of bytes: ");
        Serial.println(1);
      }

      if (xSemaphoreTake(mutex, xBlockTime) == pdTRUE) {
        int nr = (fftOut_avaiable + 1) % 3;
        if (nr == fftOut_reading) {
          nr = (nr + 1) % 3;
        }

        single_channel_process_mix(nr);  // mix channel
        single_channel_process_voc(nr);  // voc channel
        fftOut_avaiable = nr;
        xSemaphoreGive(mutex);  // Release the mutex
      }

      // Record end time
      unsigned long endTime = millis();
      unsigned long processingTime = endTime - startTime;
      audio_update_ms = processingTime;
      // audio_update_ms = 10;
      if ((1000 * BUFFER_SIZE / SAMPLE_RATE) - 1 < processingTime) {
        Serial.print(">Prc T(ms): ");
        Serial.println(processingTime);
      }
    }
  }
}

void audio_master() {
  // I2S configuration for generating clocks
  i2s_config_t i2s_config = {
      .mode =
          i2s_mode_t(I2S_MODE_MASTER |
                     I2S_MODE_RX),  // Master mode, TX only (to generate clocks)
      .sample_rate = SAMPLE_RATE,   // Set sample rate to 1 kHz
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,  // 32-bit samples (24-bit
                                                     // padded to 32 bits)
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,  // Stereo (2 channels)
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,  // I2S standard format
      .intr_alloc_flags = 1,                              // Interrupt level 1
      .dma_buf_count = 3,                                 // DMA buffer count
      .dma_buf_len = BUFFER_SIZE,                         // DMA buffer length
      .use_apll = true,
      .mclk_multiple = I2S_MCLK_MULTIPLE_256,
  };

  // I2S pin configuration
  i2s_pin_config_t pin_config = {
      .mck_io_num = I2S_MCLK_IO,  // MCLK (SCKI) pin (optional)
      .bck_io_num = I2S_BCK_IO,   // Bit clock pin
      .ws_io_num = I2S_WS_IO,     // Word select pin (LRCK)
      .data_out_num =
          I2S_PIN_NO_CHANGE,    // Data output pin (not needed, but required)
      .data_in_num = I2S_DO_IO  // No data input
  };

  // Install and start the I2S driver
  Serial.println("start");
  esp_err_t result = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  if (result == ESP_ERR_NOT_FOUND) {
    Serial.println("ESP_ERR_NOT_FOUND");
  }
  if (result != ESP_OK) {
    Serial.printf("I2S driver install failed: %d\n", result);
  } else {
    Serial.println("I2S driver installed successfully");
  }
  Serial.println("clck");

  i2s_set_pin(I2S_NUM, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM);

  preprocess_windowing(non_zero_samples);

  audio_process();
}

int display_period = 0;
int x_max = PANE_WIDTH;
// , crr_y;
// inline void UpdateDisplayHalfRes()
// {
//   // if (xSemaphoreTake(mutex, xBlockTime) == pdTRUE)
//   // {
//   int nr = fftOut_avaiable;
//   fftOut_reading = nr;
//   // xSemaphoreGive(mutex); // Release the mutex
//   x_max = min(PANE_WIDTH, SAMPLES);
//   for (int x = 0; x < x_max; x++)
//   {
//     if (x % 2 == 0)
//       crr_y = (PANE_HEIGHT * vReal[x / 2]) / 8;
//     else
//       crr_y = (PANE_HEIGHT * (vReal[x / 2] + vReal[(x / 2) + 1]) / 2) / 8;
//     crr_y = max(0, crr_y - 1);
//     crr_y = min(PANEL_HEIGHT - 1, crr_y);

//     for (int y = display_last_y_pos[x] + 1; y <= crr_y; y++)
//     {
//       dma_display->drawPixelRGB888(x, y, 200, 200, 200);
//     }
//     for (int y = display_last_y_pos[x]; y > crr_y; y--)
//     {
//       drew_background_pixel(x,y);
//       // dma_display->drawPixelRGB888(x, y, back_ground[x][y][0],
//       back_ground[x][y][1], back_ground[x][y][2]);
//     }
//     display_last_y_pos[x] = crr_y;
//   }
//   // }
// }

// inline double GetAproxymateYValue(int x)
// {
//   int max_bucket = (SAMPLES / 2) * FFT_MAX_F / (DOWNSAMPLE_RATE / 2);
//   int fft_bucket = max_bucket * x / PANE_WIDTH;
//   double sqew = 1.0 - ((1.0 * max_bucket * x / PANE_WIDTH) - fft_bucket);
//   double crr_y = (PANE_HEIGHT * (vReal[fft_bucket] * sqew)) / 8;
//   crr_y += (PANE_HEIGHT * (vReal[fft_bucket + 1] * (1.0 - sqew))) / 8;
//   return crr_y;
// }
inline int GetAproxymateYValueFFTOut(int x, int channel, float boost) {
  int max_bucket = (SAMPLES / 2) * display_max_f / (DOWNSAMPLE_RATE / 2);
  int fft_bucket = max_bucket * x / PANE_WIDTH;
  double sqew = 1.0 - ((1.0 * max_bucket * x / PANE_WIDTH) - fft_bucket);
  double crr_y =
      (PANE_HEIGHT * (fftOut[fftOut_reading][channel][fft_bucket] * sqew));
  crr_y += (PANE_HEIGHT *
            (fftOut[fftOut_reading][channel][fft_bucket + 1] * (1.0 - sqew)));
  crr_y *= boost;
  crr_y /= 8;

  int crr_y_int = crr_y;
  crr_y_int = max(0, crr_y_int - 1);
  crr_y_int = min(PANEL_HEIGHT - 1, crr_y_int);
  return crr_y_int;
}

int16_t f_left[PANE_HEIGHT];
int16_t f_right[PANE_WIDTH][PANE_HEIGHT];
inline void CalkDym(int channel) {
  // Left
  for (int y = 1; y < PANEL_HEIGHT; y++) {
    f_left[y] = 0;
    f_right[PANE_WIDTH - 1][y] == 0;
  }
  // for (int i = 0; PANE_WIDTH > i; i++)
  // {
  //   crr_y = GetAproxymateYValueFFTOut(i);;

  //   for (int y = 1; y <= crr_y; y++)
  //   {
  //     if (i > 0)
  //     {
  //       f_left[i][y] = f_left[i - 1][y] + 1;
  //     }
  //     else
  //     {
  //       f_left[i][y] = 0;
  //     }
  //   }
  //   for (int y = crr_y + 1; y < PANEL_HEIGHT; y++)
  //   {
  //     if (i > 0)
  //     {
  //       f_left[i][y] = 0;
  //     }
  //   }
  // }
  // right
  int crr_y;
  for (int i = PANE_WIDTH - 2; i >= 0; i--) {
    crr_y = GetAproxymateYValueFFTOut(i, channel, 1.0);

    for (int y = 1; y <= crr_y; y++) {
      if (i < PANE_WIDTH - 1) {
        f_right[i][y] = f_right[i + 1][y] + 1;
      }
    }

    for (int y = crr_y + 1; y < PANEL_HEIGHT; y++) {
      if (i > 0) {
        if (f_right[i][y] == 0) break;
        f_right[i][y] = 0;
      }
    }
  }
}

inline void UpdateDisplay_deactivate_channel(bool mirror, int y_start, int x,
                                             int crr_y,
                                             int* display_last_y_pos) {
  for (int y = display_last_y_pos[x]; y > crr_y; y--) {
    int display_y_flip = PANEL_HEIGHT - 1 - y_start - y;
    int display_y_normal = y_start + y;

    if (mirror) {
      drew_background_pixel(x, display_y_normal);
      display_y_flip++;
    }
    drew_background_pixel(x, display_y_flip);
  }
  display_last_y_pos[x] = crr_y;
}

int red_val, green_val, blue_val;
inline void UpdateDisplay_activate_channel(bool mirror, int y_display_start,
                                           int minimal_y, int x, int crr_y,
                                           RGB col, bool use_flame) {
  int display_x, display_y_normal, display_y_flip;
  RGB crr_col = col;
  int toYellow;
  // CalkDym(0);

  display_x = x;

  // crr_y = (PANE_HEIGHT * vReal[x]) / 8;

  // for (int y = 1; y <= crr_y; y++)
  for (int y = minimal_y; y <= crr_y; y++) {
    if (use_flame) {
      f_left[y]++;

      toYellow = min(f_left[y], f_right[x][y]) - 3;  // f_right[x][y]
      toYellow = min(toYellow, (2 * y) - 3);
      toYellow = max(toYellow, 0);
      toYellow = min(toYellow, int(flame_gradient_len));

      crr_col = mix(col, mix_flame_C_col, toYellow, flame_gradient_len);
    }

    display_y_flip = PANEL_HEIGHT - 1 - y_display_start - y;
    display_y_normal = y_display_start + y;

    // float brs = 1.0 * brightness / 50;
    // int r = brightness * (140 - (toYellow / 4)) / 50;

    if (mirror) {
      dma_display->drawPixelRGB888(display_x, display_y_normal, crr_col.r,
                                   crr_col.g, crr_col.b);
      display_y_flip++;
    }

    dma_display->drawPixelRGB888(display_x, display_y_flip, crr_col.r,
                                 crr_col.g, crr_col.b);

    // uint16_t col;
    // dma_display->drawPixel(display_x, display_y, col);
  }
  if (use_flame) {
    for (int y = crr_y + 1; PANEL_HEIGHT > y; y++) {
      f_left[y] = 0;
    }
  }
}

inline bool UpdateDisplayVar(bool mirror, int y_start, int max_y) {
  static int display_last_y_pos_mix[PANE_WIDTH];
  static int display_last_y_pos_voc[PANE_WIDTH];
  int crr_y_mix;
  int crr_y_voc;

  bool new_frame = false;
  if (xSemaphoreTake(mutex, xBlockTime) == pdTRUE) {
    if (fftOut_reading != fftOut_avaiable) {
      fftOut_reading = fftOut_avaiable;
      new_frame = true;
    }

    xSemaphoreGive(mutex);
    //============

    if (new_frame) {
      bool flame;
      if (falame_colour_enable) {
        flame = true;
        CalkDym(0);
      } else {
        flame = false;
      }

      for (int x = 0; x < x_max; x++) {
        crr_y_mix = GetAproxymateYValueFFTOut(x, 0, 1.0);

        if (enable_voc_channel && x > 32) {
          crr_y_voc = GetAproxymateYValueFFTOut(x, 1, 1.25);
        } else {
          crr_y_voc = 0;
        }

        UpdateDisplay_deactivate_channel(mirror, y_start, x,
                                         max(crr_y_mix, crr_y_voc),
                                         display_last_y_pos_mix);

        // float sat_m = 1.3;
        // red_val = 140 * sat_m;
        // green_val = 50 * sat_m;
        // blue_val = 12 * sat_m;

        if (x > 32) {
          if (crr_y_voc == 0 && display_last_y_pos_voc[x] > 0) {
            dma_display->drawPixelRGB888(
                x, PANE_HEIGHT - 1, (mix_C_col.r), (mix_C_col.g),
                (mix_C_col.b));  /// TODO add proper method so
                                 /// mirror works properly
          }
          display_last_y_pos_voc[x] = crr_y_voc;
          // UpdateDisplay_deactivate_channel(mirror, y_start, x, crr_y_voc,
          //                                  display_last_y_pos_voc);
        }

        UpdateDisplay_activate_channel(mirror, y_start, crr_y_voc + 1, x,
                                       crr_y_mix, mix_C_col, flame);

        if (x > 32) {
          UpdateDisplay_activate_channel(mirror, y_start, 1, x, crr_y_voc,
                                         voice_C_col, false);
          if (crr_y_voc > 0) {
            dma_display->drawPixelRGB888(
                x, PANE_HEIGHT - 1, voice_C_col.r, voice_C_col.g,
                voice_C_col.b);  /// TODO add proper method so
                                 /// mirror works properly
          }
        }
      }
    }
  }
  return new_frame;
}