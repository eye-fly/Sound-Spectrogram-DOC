#include <Arduino.h>

#include <vector>

// #include "display.h"
#include "menu/menu.h"
Preferences preferences;
volatile int encoderPos = 0;  // Position counter
volatile int buttonPressed = 0;
volatile int display_update_avg_ms;
volatile int display_update_max_ms;
volatile int audio_update_ms;

void show_update_time() {
  update__small_num(display_update_avg_ms, 0, 1, col_dark_grey);
  update__small_num(display_update_max_ms, 0, 7, col_dark_grey);
  update__small_num(audio_update_ms, 0, 13, col_dark_grey);
}

ListMenu* crrMenu;

ListMenu::ListMenu(String n, std::vector<Menu*> items,
                   std::function<void()> fun)
    : Menu(n), listCOntent(items), additionalDisplay(fun) {}

void ListMenu::reverse_changes() {
  for (auto* item : listCOntent) {
    if (item) {
      item->reverse_changes();
    }
  }
};
void ListMenu::restore_default() {
  for (auto* item : listCOntent) {
    if (item) {
      item->restore_default();
    }
  }
}
void ListMenu::save_changes() {
  for (auto* item : listCOntent) {
    if (item) {
      item->save_changes();
    }
  }
}

int ListMenu::items_count() { return listCOntent.size(); }
int ListMenu::y_lenght() { return 8; }

void ListMenu::short_press(ListMenu* prevList) {
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

// void long_press() { returnFun(); };

// x,y pos of upper left corrner
// void print(int x, int y) { text(Name, x, y, col_white, col_black); }
void ListMenu::zero_encoder_offset() {
  encoder_selected_item_offset = encoderPos - selected_item;
}
void ListMenu::update(int x, int y) {
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

void ListMenu::show_content(int x, int y) {
  if (is_shown) {
    update(x, y);
  } else {
    print_content(x, y);
    // TODO: add no change zone so fft wont ruin menu
    is_shown = 1;
  }
}
void ListMenu::print_content(int x, int y) {
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

  additionalDisplay();
}

// MenuItem =====================
const char* next_key() {
  static char key[4] = "0";
  size_t len = strlen(key);

  if (key[len - 1] == 'z') {
    if (len + 1 < sizeof(key)) {
      key[len] = '0';
      key[len + 1] = '\0';
    }
  } else {
    key[len - 1]++;  // Increment last char (safe)
  }
  return key;
}

MenuItem::MenuItem(String n, volatile int* ref, int val_min, int val_max,
                   int scaler, std::function<void()> fun,
                   std::function<void()> displFun)
    : Menu(n),
      val(ref),
      val_min(val_min),
      val_max(val_max),
      encoder_pulse_multiple(scaler),
      updateFun(fun),
      auxDisplayFun(displFun) {
  defoult_val = (*val);
  preferences.begin("options", true);
  pref_key = strdup(next_key());
  (*val) = preferences.getInt(pref_key, defoult_val);
  fun();
  preferences.end();
}

// storing option values ==================================
void MenuItem::reverse_changes() {
  preferences.begin("options", true);
  (*val) = preferences.getInt(pref_key, defoult_val);
  updateFun();
  preferences.end();
}
void MenuItem::restore_default() {
  (*val) = defoult_val;
  updateFun();
  // TODO: add notice that this chenge needs to be save if it's supoused to
  // last
}
void MenuItem::save_changes() {
  preferences.begin("options", false);
  Serial.println(pref_key);
  preferences.putInt(pref_key, (*val));
  preferences.end();
}
// ===============================

void MenuItem::print(int x, int y, uint16_t col) {
  text(name + " " + String(*val) + " ", x, y, col, col_black);
}
void MenuItem::print(int x, int y) { print(x, y, col_white); }
void MenuItem::print_selected(int x, int y) {
  print(x, y, col_bright_white);
  draw_arrow(x - 1, y + 1, col_bright_white, false);
}

void MenuItem::update(int x, int y) {
  volatile int new_val = encoderPos - encoder_offset;
  new_val *= encoder_pulse_multiple;
  if (new_val != 0) {
    encoder_offset = encoderPos;
    new_val = (new_val + *val < val_min) ? val_min - *val : new_val;
    new_val = (new_val + *val > val_max) ? val_max - *val : new_val;
    (*val) += new_val;
    updateFun();
    auxDisplayFun();
    print(x, y, col_bright_white);
  }
  if (frames_pressed > 2 && buttonPressed == 1) {
    diselect();
  }
  frames_pressed++;
}

void MenuItem::short_press(ListMenu* prevList) {
  // pressed = 1;
  frames_pressed = 0;
  encoder_offset = encoderPos;
  returnFun = [prevList]() {
    prevList->zero_encoder_offset();
    prevList->list_selected = 1;
  };
};

void MenuItem::diselect() {
  // pressed = 0;
  returnFun();  // TODO maybe we can just call returnFun directly
};

// PopupMenu =================
PopupMenu::PopupMenu(String name, String msg, std::function<void()> confirm)
    : Menu(name), message(msg), onConfirm(confirm) {}

void PopupMenu::update(int x, int y) {
  volatile int new_val = encoderPos - encoder_offset;
  if (new_val != 0) {
    encoder_offset = encoderPos;
    selected += new_val;
    selected = (selected > 0) ? selected : 0;
    selected =
        (selected < 2 * moves_perchange) ? selected : 2 * moves_perchange - 1;

    // updateFun();
    printPopup();
  }
  if (frames_pressed > 2 && buttonPressed == 1) {
    if (selected < moves_perchange) {
      diselect();
    } else {
      onConfirm();
      diselect();
    }
  }
  frames_pressed++;
}

void PopupMenu::short_press(ListMenu* prevList) {
  // pressed = 1;
  frames_pressed = 0;
  encoder_offset = encoderPos;
  selected = 0;
  returnFun = [prevList]() {
    prevList->zero_encoder_offset();
    prevList->list_selected = 1;
    print_back_ground();
    prevList->print_content(140, 1);
  };
  print_back_ground();
  printPopup();
};

void PopupMenu::diselect() {
  // pressed = 0;
  returnFun();  // TODO maybe we can just call returnFun directly
};

void PopupMenu::printPopup() {
  int x = BCK_W / 2;
  int y = BCK_H / 2 - 30;
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

void update_test() { update__small_num(encoderPos, 125, 48, col_dark_grey); }