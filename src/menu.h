#include <Arduino.h>
#include <Preferences.h>

#include <vector>

#include "menu_aux.h"
// #include "display.h"

// Define the GPIO pins
#define ENCODER_CLK 22
#define ENCODER_DT 21
#define ENCODER_SW 34

Preferences preferences;

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

class ListMenu;
ListMenu* crrMenu;

class Menu {
 protected:
  String name;

 public:
  std::function<void()> returnFun;

  Menu(String n) : name(n) {}
  virtual int y_lenght() { return 7; }

  // x,y pos of upper left corrner
  virtual void print(int x, int y) { text(name, x, y, col_white, col_black); }
  virtual void print_selected(int x, int y) {
    text(name, x, y, col_bright_white, col_black);
    draw_arrow(x - 1, y + 1, col_bright_white, false);
  }
  virtual void update(int x, int y) {};
  void clean_arrow(int x, int y) { draw_arrow(x - 1, y + 1, 0, true); }
  virtual void short_press(ListMenu*) {};
  virtual void reverse_changes() {};
  virtual void restore_default() {};
  virtual void save_changes() {};
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

  void reverse_changes() {
    for (auto* item : listCOntent) {
      if (item) {
        item->reverse_changes();
      }
    }
  };
  void restore_default() {
    for (auto* item : listCOntent) {
      if (item) {
        item->restore_default();
      }
    }
  }
  void save_changes() {
    for (auto* item : listCOntent) {
      if (item) {
        item->save_changes();
      }
    }
  }

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
  int defoult_val;

  bool pressed = 0;
  int frames_pressed = 0;
  int encoder_pulse_multiple = 1;  // one move of encoder is equal to this value
  int encoder_offset;

  std::function<void()> updateFun;

 public:
  MenuItem(
      String n, volatile int* ref = nullptr, int scaler = 1,
      std::function<void()> fun = []() {})
      : Menu(n), val(ref), encoder_pulse_multiple(scaler), updateFun(fun) {
    defoult_val = (*val);
    preferences.begin("options", true);
    (*val) = preferences.getInt(n.c_str(), defoult_val);
    preferences.end();
  }

  // storing option values ==================================
  void reverse_changes() {
    preferences.begin("options", true);
    (*val) = preferences.getInt(name.c_str(), defoult_val);
    preferences.end();
  }
  void restore_default() {
    (*val) = defoult_val;
    // TODO: add notice that this chenge needs to be save if it's supoused to
    // last
  }
  void save_changes() {
    preferences.begin("options", false);
    preferences.putInt(name.c_str(), (*val));
    preferences.end();
  }
  // ===============================

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

class PopupMenu : public Menu {
 private:
  String message;
  std::function<void()> onConfirm;
  bool pressed = 0;
  int frames_pressed = 0;
  int selected = 0;
  const int moves_perchange = 2;

  int encoder_offset;

 public:
  PopupMenu(String msg, std::function<void()> confirm)
      : Menu(""), message(msg), onConfirm(confirm) {}

  void update(int x, int y) override {
    volatile int new_val = encoderPos - encoder_offset;
    if (new_val != 0) {
      encoder_offset = encoderPos;
      selected += new_val;
      selected = (selected > 0) ? selected : 0;
      selected =
          (selected < 2 * moves_perchange) ? selected : 2 * moves_perchange - 1;

      // updateFun();
      print_selected(x, y);
    }
    if (frames_pressed > 2 && buttonPressed == 1) {
      if (selected < moves_perchange) {
        diselect();
      } else {
        onConfirm();
      }
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
    printPopup();
  };

  void diselect() {
    // pressed = 0;
    returnFun();  // TODO maybe we can just call returnFun directly
  };

  void printPopup() {
    int x = BCK_W / 2;
    int y = BCK_H / 2;
    // Draw popup background
    // fillRect(x, y, BCK_W - x, 24, col_black);
    // drawRect(x, y, BCK_W - x, 24, col_white);

    // Center message text
    int textX = x + (BCK_W - x - textWidth(message)) / 2;
    text(message, textX, y + 8, col_white, col_black);

    // Print options
    // TODO: add arrow for selected opt~
    if (selected < moves_perchange) {
      text("NO", BCK_W - 40, y + 20, col_bright_white, col_black);
      text("YES", x + 20, y + 20, col_white, col_black);
    } else {
      text("NO", BCK_W - 40, y + 20, col_white, col_black);
      text("YES", x + 20, y + 20, col_bright_white, col_black);
    }
  }

  // void print_selected(int x, int y) override {
  //   if (selected_item == 0) {  // YES selected
  //     text("YES", x + 20, y + 20, col_bright_white, col_black);
  //     draw_arrow(x + 15, y + 21, col_bright_white, false);
  //   } else {  // NO selected
  //     text("NO", BCK_W - 40, y + 20, col_bright_white, col_black);
  //     draw_arrow(BCK_W - 45, y + 21, col_bright_white, false);
  //   }
  // }
};

int fftTest = 200;
ListMenu soundMenu = ListMenu("sound Menu", {});
ListMenu displayMenu = ListMenu("display Menu", {});

ListMenu mainMenu = ListMenu("M", {&displayMenu, &soundMenu});

PopupMenu* createSavePopup(ListMenu* menu) {
  auto* popup = new PopupMenu("Save changes?",
                              [menu]() {  // Capture menu by value
                                menu->save_changes();
                                // Note: We don't delete popup here anymore
                              });

  // Set return function to clean up
  popup->returnFun = [popup]() { delete popup; };

  return popup;
}

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
