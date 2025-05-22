## About

This project is a **Sound Spectrogram** designed for visualizing music, with a primary focus on achieving a high refresh rate (approximately 100 Hz) and high resolution for a relatively small sound frequency range (0 Hz - 700 Hz). This allows rapid changes in sound to be effectively captured and displayed, even on inexpensive hardware.

The main hardware component utilized is the **ESP32 microcontroller**, chosen for its affordability. However, due to the poor resolution of the ESP32's internal ADC (Analog-to-Digital Converter), an external ADC was integrated to ensure higher fidelity in audio capture.  
*See diagram below for reference: ![Diagram](doc/diagram.png)*

Additionally, a rotary encoder is included for user interface interactions.

For the **HUB75 protocol** used to communicate with the LED display, the [ESP32-HUB75-MatrixPanel-DMA library](https://github.com/mrcodetastic/ESP32-HUB75-MatrixPanel-DMA) was employed.

---

### Key Features:

- **Dual-Core Usage:**  
  - Tasks are divided between two concurrent processes, enabling faster refresh rates.
  - Cores are synchronized with each other using a Mutex and form of Cyclic buffer.
  - Ensures that during long display updates or user interface handling, audio sampling remains uninterrupted witch avoids audio data loss.

- **Optimized Pixel Update:**  
  - Only pixels that change compared to previous frames are updated, greatly reducing processing load.

- **low level Opimiziation**
  - Especially important in the pixel-update task, conditional jump statements are minimized, `-O3` compiler optimization is applied, and applicable functions are defined as `inline` to improve CPU efficiency and enable better instruction pipelining and future execution.
  - (microcontroler specyfic) some function that are frequently called are explicitly sotred inside internal instruction RAM (`IRAM_ATTR`)

- **Dynamic Programming**
  - For certain simple visual effects, DP is used to significantly reduce computational complexity and improve performance.

- **Preprocessing Data**
  - When possible, Hanning window (constant values) and logarithmic scaling for display purposes (no high precision needed) are preprocessed to avoid long floating-point operations like `log` or `sin`.

- **DMA (Direct Memory Access):**  
  - Drives the display without CPU usage. The CPU only changes pixel values in RAM when necessary, while DMA handles the rest.  
  - DMA is also used to receive data from the external ADC and store it in RAM, ready for processing in the next frame.

- **Downsampling Audio Data:**  
  - Allows for smaller FFT sample sizes, optimizing performance even with the minimum sample frequency of the external ADC.  
  - A low-pass filter is used before downsampling to improve accuracy.

- **Rolling Window with Zero Padding:**  
  - Enables a high refresh rate while providing the FFT with sufficient samples for stable results.  
  - Adjustable zero padding increases resolution and offers user control to emphasize quick sounds like drum beats.  
  - Preprocessing with the Hann function optimizes performance, avoiding real-time computation slowdowns.

- **Live Adjustments via Built-in Menu:**  
  - Users can adjust parameters and immediately observe their effects.  
  - *(Menu Preview: ![Menu](doc/menu.JPG))*

---
## Project Structure
```
├── include/                     # Header files (global declarations)
│   ├── envVar.h                # Main environment variables and definitions
│   └── hardwarePins.h          # Hardware pin mappings for peripherals

├── src/                         # Main application code
│   ├── main.cpp                # Entry point for PlatformIO + some audio processing (to be moved)
│   ├── display.cpp             # Implementation of LED matrix initialization and basic operations
│
│   ├── util/                   # Utility functions
│   │   ├── col.cpp            # HSL/RGB conversion and color helper functions
│   │   └── util.cpp           # Math helper functions
│
│   ├── menu/                   # User interface/menu implementation
│
│   └── fft/                    # FFT display and related optimizations
│       ├── display.cpp        # Methods for displaying FFT on LED matrix
│       └── cache.cpp          # Optimization routines for FFT/display rendering

├── platformio.ini              # PlatformIO build configuration
```
### Prototype:

*Working Prototype: ![Prototype](doc/prototype.JPG)*

---

## Showcase

[![Sound Spectrogram Showcase](https://img.youtube.com/vi/mPfA6HythuQ/0.jpg)](https://youtu.be/mPfA6HythuQ "Watch the Showcase on YouTube")
