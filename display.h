#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include "config.h"

// Display params
#define DISPLAY_COLS      16
#define DISPLAY_ROWS      2
#define DEG_SYM           (unsigned char)223

enum UIState {
  HOME,
  MAIN_MENU,
  PID_MENU,
  EDIT_TEMP,
  CALIBRATE_RTD,
  EDIT_KP,
  EDIT_KI,
  EDIT_KD,
  EDIT_FF,
  AUTOTUNE
};

enum MenuAction {
  ACTION_NONE,
  ACTION_EDIT_TEMP,
  ACTION_EDIT_CALIBRATION,
  ACTION_RESET,
  ACTION_OPEN_PID_MENU,
  ACTION_AUTOTUNE,
  ACTION_EDIT_KP,
  ACTION_EDIT_KI,
  ACTION_EDIT_KD,
  ACTION_EDIT_FF
};

struct MenuItem {
  const char* icon;   // 1-2 chars for top bar (e.g., "T", "R", "P", "Kp")
  const char* label;  // Up to 16 chars for bottom row (e.g., "Set Temp")
  MenuAction action;      // What action to trigger when selected
};

class Display {
public:
  Display();
  bool begin();
  void update(const MachineState& state, const PIDGains& pid);
  void handleInput(int delta, bool shortClick, bool longClick, MachineState& state, PIDGains& pid);

private:
  hd44780_I2Cexp lcd;
  UIState currentMode;
  int menuIndex;
  float editBuffer;
  
  const char* pad(const char* buffer, uint8_t length = DISPLAY_COLS);
  void renderHome(const MachineState& state);
  void renderMenu(const MenuItem* items, uint8_t count, uint8_t selectedIndex);
  void renderEdit(const char* label, float value, const char* unit, bool isTemp);
  void handleMenuSelection(int delta, bool act, const MenuItem* items, uint8_t count, MachineState& state, PIDGains& pid);
};

#endif