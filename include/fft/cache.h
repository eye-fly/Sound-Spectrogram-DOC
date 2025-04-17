#pragma once

#include <esp_heap_caps.h>

extern float* s_sqew_table_psram;

void initPSRAMTable();
uint8_t CalAproxymateYValueFFTOutPreComp(int x, int channel, float boost);