#pragma once
#include <Preferences.h>

#include "display.h"
#include "menu/menu_aux.h"

// #include "menu/menu_main.h"

// Define the GPIO pins
#define ENCODER_CLK 22
#define ENCODER_DT 21
#define ENCODER_SW 34

extern Preferences preferences;
extern volatile int encoderPos;
extern volatile int buttonPressed;

extern volatile int display_update_avg_ms;
extern volatile int display_update_max_ms;
extern volatile int audio_update_ms;
// static void text(String text, int x, int y, uint16_t col, uint16_t back_col);
void update__small_num(uint n, int x, int y, uint16_t col);

class ListMenu;
extern ListMenu* crrMenu;

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
  std::vector<Menu*> listCOntent;
  bool is_shown = false;
  unsigned int selected_item = 0;
  bool list_selected = false;  // enable disable scrolling list of items
  int encoder_selected_item_offset = 0;
  std::function<void()> returnFun;
  std::function<void()> additionalDisplay;

  ListMenu(
      String n, std::vector<Menu*> items,
      std::function<void()> additionalDisplay = []() {});

  void reverse_changes();
  void restore_default();
  void save_changes();
  int items_count();
  int y_lenght() override;

  void short_press(ListMenu* prevList) override;
  void zero_encoder_offset();
  void update(int x, int y) override;
  void show_content(int x, int y);
  void print_content(int x, int y);
};

extern volatile int def;  // Declaration of external variable

class MenuItem : public Menu {
 private:
  volatile int* val;
  int defoult_val;
  int val_min;
  int val_max;
  bool pressed;
  int frames_pressed;
  int encoder_pulse_multiple;
  int encoder_offset;
  std::function<void()> updateFun;
  std::function<void()> auxDisplayFun;
  const char* pref_key;

 public:
  MenuItem(
      String n, volatile int* ref, int val_min = 0, int val_max = 256,
      int scaler = 1, std::function<void()> fun = []() {},
      std::function<void()> displFun = []() {});

  // Value management
  void reverse_changes();
  void restore_default();
  void save_changes();

  // Display methods
  void print(int x, int y, uint16_t col);
  void print(int x, int y) override;
  void print_selected(int x, int y) override;
  void clean_arrow(int x, int y);

  // Interaction methods
  void update(int x, int y) override;
  void short_press(ListMenu* prevList) override;
  void diselect();
};

class PopupMenu : public Menu {
 private:
  String message;
  std::function<void()> onConfirm;
  bool pressed;
  int frames_pressed;
  int selected;
  const int moves_perchange = 2;
  int encoder_offset;

 public:
  PopupMenu(String name, String msg, std::function<void()> confirm);

  void update(int x, int y) override;
  void short_press(ListMenu* prevList) override;
  void diselect();
  void printPopup();
};

void show_update_time();