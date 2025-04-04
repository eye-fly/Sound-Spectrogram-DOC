#include "menu/menu_main.h"

#include "util.h"

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

void menu_setup() {
  mainMenu.listCOntent.push_back(new PopupMenu(
      "Save changes", "Save changes?", []() { mainMenu.save_changes(); }));
  mainMenu.listCOntent.push_back(
      new PopupMenu("Reverse changes", "Reverse changes?",
                    []() { mainMenu.reverse_changes(); }));
  mainMenu.listCOntent.push_back(
      new PopupMenu("Restore default", "Restore default?",
                    []() { mainMenu.restore_default(); }));

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
      {new ColorSelectMenu("center col", 55, 170, 80, mix_flame_C_col,
                           &mix_C_col),
       new MenuItem("en eff", &falame_colour_enable, 1,
                    []() { prindColourSample(&mix_C_col); }),
       new MenuItem(
           "eff grad", &flame_gradient_len, 1,
           []() {
             prindColourSample(&mix_C_col);
           })},  // TODO needs to be limited to >0 or rash when dividing by 0
      []() { prindColourSample(&mix_C_col); }));

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
