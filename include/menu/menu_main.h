#pragma once
#include "envVar.h"
#include "menu/color.h"
#include "menu/menu.h"

extern ListMenu soundMenu;
extern ListMenu displayMenu;
extern ListMenu mainMenu;

inline PopupMenu* createPopup(std::function<void()> onConfirm) {
  auto* popup = new PopupMenu("Save chnges", "Save changes?", onConfirm);
  return popup;
}

void menu_init();
void menu_setup();
void update_menu();

void IRAM_ATTR handleDT();
void IRAM_ATTR handleCLK();

extern volatile bool buttonIsPressed;
// volatile bool buttonPressedLong = false;
// ISR for button press
void IRAM_ATTR checkButton();