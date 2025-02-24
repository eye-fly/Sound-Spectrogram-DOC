#include <Arduino.h>

#include <vector>
// #include "display.h"

// Define the GPIO pins
#define ENCODER_CLK 22
#define ENCODER_DT 21
#define ENCODER_SW 34

volatile int display_update_avg_ms;
volatile int display_update_max_ms;
volatile int audio_update_ms;
static void text(String text, int x, int y, uint16_t col, uint16_t back_col);
void update__small_num(uint n, int x, int y, uint16_t col);

void show_update_time() {
  update__small_num(display_update_avg_ms, 0, 1, col_dark_grey);
  update__small_num(display_update_max_ms, 0, 7, col_dark_grey);
  update__small_num(audio_update_ms, 0, 13, col_dark_grey);
}

volatile int encoderPos = 0;  // Position counter
volatile bool half_rot = 0;
volatile bool dir = 0;
void IRAM_ATTR handleCLK() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);
  if (!clkState) {  // falling
    if (!half_rot) {
      if (!dtState) {
        encoderPos--;
        half_rot = 1;
        dir = 0;
      }
    }
  } else {  // rising
    if (half_rot)
      if ((dir) && (dtState)) {
        half_rot = 0;
        encoderPos--;
      } else if (!dir && dtState) {
        half_rot = 0;
      }
  }
}

void IRAM_ATTR handleDT() {
  int clkState = digitalRead(ENCODER_CLK);
  int dtState = digitalRead(ENCODER_DT);
  if (!dtState) {  // falling
    if (!half_rot) {
      if (!clkState) {
        encoderPos++;
        half_rot = 1;
        dir = 1;
      }
    }
  } else {  // rising
    if (half_rot)
      if ((!dir) && (clkState)) {
        half_rot = 0;
        encoderPos++;
      } else if (dir && clkState) {
        half_rot = 0;
      }
  }
}

// Define debounce interval (in milliseconds)
const unsigned long debounceDelay = 10;
// Button state
volatile unsigned long lastButtonTime = 0;
volatile bool buttonIsPressed = false;
volatile int buttonPressed = 0;
// volatile bool buttonPressedLong = false;
// ISR for button press
void IRAM_ATTR checkButton() {
  unsigned long crr = millis();
  int buttState = !digitalRead(ENCODER_SW);

  buttonPressed = 0;
  if (buttState != buttonIsPressed) {
    if (buttState) {
      lastButtonTime = crr;
    }

    if (!buttState && (crr - lastButtonTime > 250)) {
      buttonPressed = 2;
    } else if (!buttState && (crr - lastButtonTime > 50)) {
      buttonPressed = 1;
    }
    buttonIsPressed = buttState;
  }
}

void menu_init() {
  // Set up encoder pins
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), handleCLK, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT), handleDT, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(ENCODER_DT), handleDT_fall,
  // FALLING); attachInterrupt(digitalPinToInterrupt(ENCODER_DT),
  // handleDT_rise, RISING);

  //   attachInterrupt(digitalPinToInterrupt(ENCODER_SW), handleButton,
  //   FALLING);
}

// Define number patterns as 5x3 grids
const int n_rows = 5;
const int n_cols = 3;
const char numbers[10][n_rows][n_cols] = {
    {{'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', ' ', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'}},  // 0
    {{' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'}},  // 1
    {{'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'},
     {'1', ' ', ' '},
     {'1', '1', '1'}},  // 2
    {{'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'}},  // 3
    {{'1', ' ', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'}},  // 4
    {{'1', '1', '1'},
     {'1', ' ', ' '},
     {'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'}},  // 5
    {{'1', '1', '1'},
     {'1', ' ', ' '},
     {'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'}},  // 6
    {{'1', '1', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'},
     {' ', ' ', '1'}},  // 7
    {{'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'}},  // 8
    {{'1', '1', '1'},
     {'1', ' ', '1'},
     {'1', '1', '1'},
     {' ', ' ', '1'},
     {'1', '1', '1'}}  // 9
};

void update_single_dig(int n, int x, int y, uint16_t col) {
  for (int i = 0; i < n_rows; i++) {
    for (int j = 0; j < n_cols; j++) {
      if (numbers[n][i][j] == '1') {
        dma_display->drawPixel(x + j, y + i, col);
      } else {
        drew_background_pixel(x + j, y + i);
      }
    }
  }
}

void erase_dig(int x, int y) {
  for (int i = 0; i < n_rows; i++) {
    for (int j = 0; j < n_cols; j++) {
      // if (numbers[1][i][j] == '1') {
      drew_background_pixel(x + j, y + i);
      // }
    }
  }
}
void update__small_num(uint n, int x, int y, uint16_t col) {
  int dig_p = 0;
  int diel = 10;
  int d;

  while (diel <= n) diel *= 10;
  diel /= 10;
  while (diel >= 1) {
    d = n / diel;
    d %= 10;
    update_single_dig(d, x, y, col);
    x += n_cols + 1;
    n %= diel;
    diel /= 10;
    dig_p++;
    // if (n < 10) break;
  }
  // if (dig_p == 0)
  //   update_single_dig(0, x, y, col);
  // else
  erase_dig(x, y);
}

const uint8_t arrow[7] = {
    0b00100,  // Row 0
    0b01110,  // Row 1
    0b11111,  // Row 2
    0b00100,  // Row 3
    0b00100,  // Row 4
    0b00100,  // Row 5
    0b00100   // Row 6
};
static void draw_arrow(int x, int y, uint16_t color, bool clear) {
  for (int row = 0; row < 7; row++) {
    for (int col = 4; col >= 0; col--) {  // 5 columns (bits 4 to 0)
      if (arrow[row] & (1 << col)) {      // Check if the bit is set
        if (clear) {
          drew_background_pixel(x - row, y + col);
        } else {
          dma_display->drawPixel(x - row, y + col, color);
        }
      }
    }
  }
}

class ListMenu;
ListMenu* crrMenu;

class Menu {
 protected:
  String name;

 public:
  std::function<void()> returnFun;

  Menu(String n) : name(n) {}
  virtual int y_lenght() = 0;

  // x,y pos of upper left corrner
  virtual void print(int x, int y) { text(name, x, y, col_white, col_black); }
  virtual void print_selected(int x, int y) {
    text(name, x, y, col_bright_white, col_black);
    draw_arrow(x - 1, y + 1, col_bright_white, false);
  }
  virtual void update(int x, int y) {};
  void clean_arrow(int x, int y) { draw_arrow(x - 1, y + 1, 0, true); }
  virtual void short_press(ListMenu*) {};
};

class ListMenu : public Menu {
 private:
  int crr_y_offset = 0;
  int last_selected = 0;

 public:
  ListMenu(String n, std::vector<Menu*> items) : Menu(n), listCOntent(items) {}

  std::vector<Menu*> listCOntent;
  bool is_shown = 0;
  unsigned int selected_item = 0;
  bool list_selected = 0;  // enable disable scrolling list of items
  int encoder_selected_item_offset = 0;

  int items_count() { return listCOntent.size(); }
  int y_lenght() override { return 8; }

  void short_press(ListMenu* prevList) override {
    /// TODO need to call print content onece and then update but also consider
    /// function update_menu;
    list_selected = 1;
    is_shown = 0;
    crrMenu = this;
    print_back_ground();
    returnFun = [prevList]() {
      crrMenu = prevList;
      prevList->zero_encoder_offset();
      prevList->list_selected = 1;
      prevList->is_shown = 0;
      print_back_ground();
    };
  };

  void long_press() { returnFun(); };

  // x,y pos of upper left corrner
  // void print(int x, int y) { text(Name, x, y, col_white, col_black); }
  void zero_encoder_offset() {
    encoder_selected_item_offset = encoderPos - selected_item;
  }
  void update(int x, int y) override {
    if (list_selected) {
      if (buttonPressed == 1) {
        listCOntent[selected_item]->short_press(this);
        list_selected = 0;
        return;  // we might be changing menu so we sikip rest of theupdate this
                 // turn
      }
      selected_item = encoderPos - encoder_selected_item_offset;
      selected_item %= items_count();
    }

    if (buttonPressed == 2) {  // LONGpress exit form menu
      returnFun();
      return;  // we dont won't to print list enmore
    }

    if (last_selected != selected_item) {
      for (int i = crr_y_offset; i < listCOntent.size(); i++) {
        if (i == last_selected) {
          listCOntent[last_selected]->clean_arrow(x, y);
          listCOntent[last_selected]->print(x,
                                            y);  // to change to normal colour
        }
        if (i == selected_item) {
          listCOntent[selected_item]->print_selected(x, y);
        }
        y += 1 + listCOntent[i]->y_lenght();
      }
      last_selected = selected_item;
    }

    if (!list_selected) {
      for (int i = crr_y_offset; i < listCOntent.size(); i++) {
        if (i == selected_item) {
          listCOntent[selected_item]->update(x, y);
        }
        y += 1 + listCOntent[i]->y_lenght();
      }
    }
  }

  void show_content(int x, int y) {
    if (is_shown) {
      update(x, y);
    } else {
      print_content(x, y);
      // TODO: add no change zone so fft wont ruin menu
      is_shown = 1;
    }
  }
  void print_content(int x, int y) {
    for (int i = crr_y_offset; i < listCOntent.size(); i++) {
      if (last_selected != selected_item) {
        listCOntent[i]->clean_arrow(x, y);
      }
      if (selected_item == i) {
        listCOntent[i]->print_selected(x, y);
      } else {
        listCOntent[i]->print(x, y);
      }
      y += 1 + listCOntent[i]->y_lenght();
    }
    last_selected = selected_item;
  }
};

volatile int def = 0;
class MenuItem : public Menu {
 private:
  // String Name;
  volatile int* val = &def;

  bool pressed = 0;
  int frames_pressed = 0;
  int encoder_pulse_multiple = 1;  // one move of encoder is equal to this value
  int encoder_offset;

  std::function<void()> updateFun = []() {};

 public:
  MenuItem(String n) : Menu(n) {}
  MenuItem(String n, volatile int* ref) : Menu(n), val(ref) {}
  MenuItem(String n, volatile int* ref, int scaler)
      : Menu(n), val(ref), encoder_pulse_multiple(scaler) {}
  MenuItem(String n, volatile int* ref, std::function<void()> fun)
      : Menu(n), val(ref), updateFun(fun) {}
  MenuItem(String n, volatile int* ref, int scaler, std::function<void()> fun)
      : Menu(n), val(ref), updateFun(fun), encoder_pulse_multiple(scaler) {}
  int y_lenght() override { return 7; }

  void print(int x, int y, uint16_t col) {
    text(name + " " + String(*val) + " ", x, y, col, col_black);
  }
  void print(int x, int y) override { print(x, y, col_white); }
  void print_selected(int x, int y) override {
    print(x, y, col_bright_white);
    draw_arrow(x - 1, y + 1, col_bright_white, false);
  }

  void update(int x, int y) override {
    volatile int new_val = encoderPos - encoder_offset;
    new_val *= encoder_pulse_multiple;
    if (new_val != 0) {
      encoder_offset = encoderPos;
      (*val) += new_val;
      updateFun();
      print(x, y, col_bright_white);
    }
    if (frames_pressed > 2 && buttonPressed == 1) {
      diselect();
    }
    frames_pressed++;
  }

  void short_press(ListMenu* prevList) override {
    // pressed = 1;
    frames_pressed = 0;
    encoder_offset = encoderPos;
    returnFun = [prevList]() {
      prevList->zero_encoder_offset();
      prevList->list_selected = 1;
    };
  };

  void diselect() {
    // pressed = 0;
    returnFun();  // TODO maybe we can just call returnFun directly
  };
};

int fftTest = 200;
ListMenu soundMenu = ListMenu("sound Menu", {});
ListMenu displayMenu = ListMenu("display Menu", {});

ListMenu mainMenu = ListMenu("M", {&displayMenu, &soundMenu});

void update_test() { update__small_num(encoderPos, 125, 48, col_dark_grey); }

bool start = 0;
bool show_menu = 0;
void update_menu() {
  if (!start) {
    crrMenu = &mainMenu;
    crrMenu->list_selected = 1;
    crrMenu->returnFun = []() {
      show_menu = 0;
      mainMenu.is_shown = 0;
      print_back_ground();  // TODO: maybe change to someting faster
      drewLine();
    };

    start = 1;
  }
  checkButton();

  // static int encoder_selected_offset = encoderPos - crrMenu.selected_item;
  // crrMenu.selected_item = encoderPos - encoder_selected_offset;
  // crrMenu.selected_item %= crrMenu.items_count();
  if (show_menu) {
    update__small_num(buttonPressed, 100, 10, col_white);
    update__small_num(buttonIsPressed + 1, 100, 16, col_white);
    crrMenu->show_content(140, 1);
  } else if (buttonPressed == 1) {
    show_menu = 1;
  }
}

static void text(String text, int x, int y, uint16_t col, uint16_t bac_col) {
  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    dma_display->drawChar(x, y, c, col, bac_col, 1);
    x += 6;
  }
}
