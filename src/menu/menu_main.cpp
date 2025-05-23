#include "menu/menu_main.h"

#include "util/util.h"

ListMenu soundMenu = ListMenu("sound Menu", {});
ListMenu displayMenu = ListMenu("display Menu", {});

ListMenu mainMenu = ListMenu("M", {&displayMenu, &soundMenu});

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
// TODO: add dim line colur so its acts dimnamicly (but check how it affect
// preformance)

template <typename T>
void addMenuItem(
    ListMenu& menu, const char* title, volatile T* value, T min, T max,
    T step = 1, std::function<void()> callback = []() {}) {
  menu.listCOntent.push_back(
      new MenuItem(title, value, min, max, step, callback));
}
template <typename T>
void addMenuItemOnOff(
    ListMenu& menu, const char* title, volatile T* value, T min, T max,
    T step = 1, std::function<void()> callback = []() {}) {
  menu.listCOntent.push_back(new MenuItem(title, value, min, max, step,
                                          callback, []() {}, {"Off", "On"}));
}

void setupMainMenu() {
  mainMenu.listCOntent.push_back(new PopupMenu(
      "Save changes", "Save changes?", []() { mainMenu.save_changes(); }));
  mainMenu.listCOntent.push_back(
      new PopupMenu("Reverse changes", "Reverse changes?",
                    []() { mainMenu.reverse_changes(); }));
  mainMenu.listCOntent.push_back(
      new PopupMenu("Restore default", "Restore default?",
                    []() { mainMenu.restore_default(); }));
}

void setupSoundMenu() {
  // FFT and volume settings
  addMenuItem(soundMenu, "FFT non0 samp", &non_zero_samples, 2,
              MAX_NON_ZERO_SAMPLES, 5,
              []() { preprocess_windowing(non_zero_samples); });
  addMenuItem(soundMenu, "volume anj", &volue_adjustment, 1, 100);
  addMenuItem(soundMenu, "use log scale", &use_log_scale, 0, 3);
  addMenuItem(soundMenu, "max f", &display_max_f, 200, 1000, 25);
  addMenuItemOnOff(soundMenu, "en voc ch", &enable_voc_channel, 0, 1);
}

void setupDisplayMenu() {
  // Flame effect submenu
  auto flameSubMenu = new ListMenu(
      "flame eff opt",
      {new ColorSelectMenu("center col", 41, 198, 89, mix_flame_C_col),
       new MenuItem(
           "en eff", &falame_colour_enable, 0, 1, 1, []() {},
           []() { prindColourSample(&mix_C_col); }, {"Off", "On"}),
       new MenuItem(
           "eff grad", &flame_gradient_len, 1, 20, 1, []() {},
           []() { prindColourSample(&mix_C_col); }),
       new MenuItem(
           "eff offset", &flame_gradd_offset, 0, 10, 1, []() {},
           []() { prindColourSample(&mix_C_col); })},
      []() { prindColourSample(&mix_C_col); });

  displayMenu.listCOntent.push_back(flameSubMenu);

  // Display settings
  addMenuItem(displayMenu, "brightness", &brightness, 1, 255, 8,
              []() { matrix->setBrightness8(brightness); });
  addMenuItemOnOff(displayMenu, "miror", &miror, 0, 1, 1, []() {
    if (miror)
      drewLine(PANE_HEIGHT / 2);
    else
      drewLine(PANE_HEIGHT - 1);
    print_back_ground();
  });

  // Color settings
  displayMenu.listCOntent.push_back(
      new ColorSelectMenu("voc col", 99, 175, 89,
                          voice_C_col));  // TODO: change so correct
  // (voice)colur is displayd (notmix)
  displayMenu.listCOntent.push_back(new ColorSelectMenu(
      "mix col", 5, 222, 112, mix_C_col, []() { drewLine(); }));
}

void menu_setup() {
  setupMainMenu();
  setupSoundMenu();
  setupDisplayMenu();
}

bool start = 0;
bool show_menu = 0;
void update_menu() {
  if (!start) {
    crrMenu = &mainMenu;
    crrMenu->list_selected = 1;
    crrMenu->returnFun = []() {
      show_menu = 0;
      mainMenu.is_shown = 0;
      mainMenu.zero_encoder_offset();
      mainMenu.list_selected = 1;

      print_back_ground();  // TODO: maybe change to someting faster
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
