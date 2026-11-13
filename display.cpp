#include "display.h"

const MenuItem MAIN_MENU_ITEMS[] = {
  { "T", "Set Temp",      ACTION_EDIT_TEMP        },
  { "C", "Calibrate RTD", ACTION_EDIT_CALIBRATION },
  { "P", "Set PID Gains", ACTION_OPEN_PID_MENU    }
};
const uint8_t MAIN_MENU_COUNT = 3;

// PID Submenu Definitions
const MenuItem PID_MENU_ITEMS[] = {
  { "A",  "Autotune", ACTION_AUTOTUNE },
  { "Kp", "Set Kp",   ACTION_EDIT_KP  },
  { "Ki", "Set Ki",   ACTION_EDIT_KI  },
  { "Kd", "Set Kd",   ACTION_EDIT_KD  }
};
const uint8_t PID_MENU_COUNT = 4;

Display::Display() : currentMode(HOME), menuIndex(0), editBuffer(0.0f) {}

bool Display::begin() {
  int status = lcd.begin(DISPLAY_COLS, DISPLAY_ROWS);
  if (status != 0) {
    return false; // LCD initialization failed
  }
  lcd.backlight();
  return true;
}

void Display::update(const MachineState& state, const PIDGains& pid) {
  // CRITICAL SAFETY OVERRIDE: Show hardware fault code if present
  if (state.sensorFaultCode != 0) {
    lcd.setCursor(0, 0);
    lcd.print(pad("RTD FAULT DETECT!"));
    char errBuf[DISPLAY_COLS + 1];
    snprintf(errBuf, sizeof(errBuf), "Err Code: 0x%02X", state.sensorFaultCode);
    lcd.setCursor(0, 1);
    lcd.print(pad(errBuf));
    return;
  }

  // Normal UI Rendering based on active state
  switch (currentMode) {
    case HOME:
      renderHome(state);
      break;
    case MAIN_MENU:
      renderMenu(MAIN_MENU_ITEMS, MAIN_MENU_COUNT, menuIndex);
      break;
    case PID_MENU:
      renderMenu(PID_MENU_ITEMS, PID_MENU_COUNT, menuIndex);
      break;
    case EDIT_TEMP:
      renderEdit(state.isSteamMode ? "Edit Steam SV:" : "Edit Brew SV:", editBuffer, "C", true);
      break;
    case CALIBRATE_RTD:
      renderEdit("Current Temp:", editBuffer, "C", 1);
      break;
    case EDIT_KP:
      renderEdit(state.isSteamMode ? "Set Steam Kp" : "Set Brew Kp", editBuffer, "", false);
      break;
    case EDIT_KI:
      renderEdit(state.isSteamMode ? "Set Steam Ki" : "Set Brew Ki", editBuffer, "", false);
      break;
    case EDIT_KD:
      renderEdit(state.isSteamMode ? "Set Steam Kd" : "Set Brew Kd", editBuffer, "", false);
      break;
    case AUTOTUNE:
      lcd.setCursor(0, 0);
      lcd.print(pad("AUTOTUNE ACTIVE"));
      lcd.setCursor(0, 1);
      lcd.print(pad("Hold to Abort..."));
      break;
  }
}

void Display::renderHome(const MachineState& state) {
  char header[DISPLAY_COLS + 1];
  if (state.isSteamMode) {
    snprintf(header, sizeof(header), "STEAM: %5.1f%cC", state.steamSetpoint, DEG_SYM);
  } else if (state.isPumpOn) {
    unsigned long elapsedSec = (millis() - state.shotStartTime) / 1000;
    snprintf(header, sizeof(header), "BREW Time: %02lus", elapsedSec);
  } else {
    snprintf(header, sizeof(header), "BREW: %5.1f%cC", state.brewSetpoint, DEG_SYM);
  }

  lcd.setCursor(0, 0);
  lcd.print(pad(header));

  char data[DISPLAY_COLS + 1];
  snprintf(data, sizeof(data), "Cur: %5.1f%cC", state.currentTemp, DEG_SYM);
  lcd.setCursor(0, 1);
  lcd.print(pad(data));
}

void Display::renderMenu(const MenuItem* items, uint8_t count, uint8_t selectedIndex) {
  char topRow[17];
  uint8_t pos = 0;

  for (uint8_t i = 0; i < count; i++) {
    const char* sym = items[i].icon;
    uint8_t symLen = strlen(sym);

    if (pos + symLen + 1 <= 16) {
      topRow[pos++] = (i == selectedIndex) ? '>' : ' ';
      memcpy(&topRow[pos], sym, symLen);
      pos += symLen;
    }

    // Add a 1-space gap between items if there's room
    if (i < count - 1 && pos < 16) {
      topRow[pos++] = ' ';
    }
  }
  topRow[pos] = '\0';

  // Row 0: Icon Bar
  lcd.setCursor(0, 0);
  lcd.print(pad(topRow));

  // Row 1: Selected Item Description
  lcd.setCursor(0, 1);
  lcd.print(pad(items[selectedIndex].label));
}

void Display::renderEdit(const char* label, float value, const char* unit, bool isTemp) {
  lcd.setCursor(0, 0);
  lcd.print(pad(label));

  char targetBuf[DISPLAY_COLS + 1];
  if (isTemp) {
    snprintf(targetBuf, sizeof(targetBuf), "     > %.1f%c%s", value, (char)223, unit);
  } else {
    snprintf(targetBuf, sizeof(targetBuf), "     > %.1f %s", value, unit);
  }
  lcd.setCursor(0, 1);
  lcd.print(pad(targetBuf));
}

void Display::handleInput(int delta, bool shortClick, bool longClick, MachineState& state, PIDGains& pid) {
  // Long press always acts as a universal cancel / back action
  if (longClick) {
    if (currentMode == EDIT_KP || currentMode == EDIT_KI || currentMode == EDIT_KD || currentMode == AUTOTUNE) {
      currentMode = PID_MENU;
      menuIndex = 0;
    } else {
      currentMode = HOME;
    }
    return;
  }

  switch (currentMode) {
    case HOME:
      // clicking from Home transitions directly to the menu
      if (shortClick) {
        currentMode = MAIN_MENU;
        menuIndex = 0;
      }
      // rotating from Home jumps directly into temperature editing
      else if (delta != 0) {
        currentMode = EDIT_TEMP;
        editBuffer = state.isSteamMode ? state.steamSetpoint : state.brewSetpoint;
        // Apply the first rotation delta immediately so it doesn't get lost
        editBuffer += delta * 0.5f;
      }
      break;

    case MAIN_MENU:
      handleMenuSelection(delta, shortClick, MAIN_MENU_ITEMS, MAIN_MENU_COUNT, state, pid);
      break;

    case PID_MENU:
      handleMenuSelection(delta, shortClick, PID_MENU_ITEMS, PID_MENU_COUNT, state, pid);
      break;

    case EDIT_TEMP:
      if (delta != 0) editBuffer += (delta * 0.5f);
      if (shortClick) {
        if (state.isSteamMode) state.steamSetpoint = editBuffer;
        else state.brewSetpoint = editBuffer;
        saveSettings(state, pid);
        currentMode = HOME;
      }
      break;

    case CALIBRATE_RTD:
      if (delta != 0) editBuffer += (delta * 0.1f);
      if (shortClick) {
        // Adjust offset by the difference between user's reference temp and current reading
        state.rtdOffset += (editBuffer - state.currentTemp);
        saveSettings(state, pid);
        currentMode = HOME;
      }
      break;

    case EDIT_KP:
      if (delta != 0) editBuffer += (delta * 0.1f);
      if (shortClick) { 
        if (state.isSteamMode) pid.steamKp = editBuffer;
        else pid.brewKp = editBuffer;
        saveSettings(state, pid);
        currentMode = PID_MENU; }
      break;

    case EDIT_KI:
      if (delta != 0) editBuffer += (delta * 0.1f);
      if (shortClick) { 
        if (state.isSteamMode) pid.steamKi = editBuffer;
        else pid.brewKi = editBuffer;
        saveSettings(state, pid);
        currentMode = PID_MENU; }
      break;

    case EDIT_KD:
      if (delta != 0) editBuffer += (delta * 0.1f);
      if (shortClick) { 
        if (state.isSteamMode) pid.steamKd = editBuffer;
        else pid.brewKd = editBuffer;
        saveSettings(state, pid);
        currentMode = PID_MENU; }
      break;

    default:
      // Autotune cancellation handled by longClick above
      break;
  }
}

// Helper to handle menu navigation and selection generically
void Display::handleMenuSelection(int delta, bool act, const MenuItem* items, uint8_t count, MachineState& state, PIDGains& pid) {
  if (delta != 0) {
    menuIndex = ((menuIndex + delta) % count + count) % count;
    return;
  }
  
  if (act) {
    switch (items[menuIndex].action) {
      case ACTION_EDIT_TEMP:
        editBuffer = state.isSteamMode ? state.steamSetpoint : state.brewSetpoint;
        currentMode = EDIT_TEMP;
        break;
      case ACTION_EDIT_CALIBRATION:
        editBuffer = state.currentTemp;
        currentMode = CALIBRATE_RTD;
        break;
      case ACTION_OPEN_PID_MENU:
        currentMode = PID_MENU;
        menuIndex = 0; // Reset index for the new menu
        break;
      case ACTION_AUTOTUNE:
        currentMode = AUTOTUNE;
        break;
      case ACTION_EDIT_KP:
        editBuffer = state.isSteamMode ? pid.steamKp : pid.brewKp;
        currentMode = EDIT_KP;
        break;
      case ACTION_EDIT_KI:
        editBuffer = state.isSteamMode ? pid.steamKi : pid.brewKi;
        currentMode = EDIT_KI;
        break;
      case ACTION_EDIT_KD:
        editBuffer = state.isSteamMode ? pid.steamKd : pid.brewKd;
        currentMode = EDIT_KD;
        break;
      default:
        break;
    }
  }
}

const char* Display::pad(const char* input, uint8_t length) {
  static char buffer[DISPLAY_COLS + 1];
  uint8_t i = 0;

  // Copy characters from input up to the max length or end of string
  while (input[i] != '\0' && i < length) {
    buffer[i] = input[i];
    i++;
  }
  
  // Pad the remaining slots with spaces if input was too short
  while (i < length) {
    buffer[i++] = ' ';
  }
  
  // Ensure strict null-termination at the exact display edge
  buffer[length] = '\0';
  
  return buffer;
}