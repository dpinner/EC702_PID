#include "config.h"

void loadSettings(MachineState& state, PIDGains& pid) {
  SettingsEEPROM saved;
  EEPROM.get(EEPROM_START_ADDR, saved);

  // Check if EEPROM has been initialized before with our magic byte
  if (saved.magic != EEPROM_MAGIC_BYTE) {
    // Fall back to defaults if uninitialized[cite: 2]
    state.brewSetpoint = DEFAULT_BREW_SETPOINT;
    state.steamSetpoint = DEFAULT_STEAM_SETPOINT;
    state.rtdOffset = DEFAULT_TEMP_OFFSET;
    pid.brewKp = DEFAULT_KP;
    pid.steamKp = DEFAULT_KP;
    pid.brewKi = DEFAULT_KI;
    pid.steamKi = DEFAULT_KI;
    pid.brewKd = DEFAULT_KD;
    pid.steamKd = DEFAULT_KD;
    pid.brewFF = DEFAULT_BREW_FF;
    pid.steamFF = DEFAULT_STEAM_FF;
    saveSettings(state, pid); // Write initial valid state to EEPROM
  } else {
    // Load persisted values into active runtime structs
    state.brewSetpoint = saved.brewSetpoint;
    state.steamSetpoint = saved.steamSetpoint;
    state.rtdOffset = saved.rtdOffset;
    pid = saved.pid;
  }
}

void saveSettings(const MachineState& state, const PIDGains& pid) {
  SettingsEEPROM dataToSave;
  dataToSave.magic = EEPROM_MAGIC_BYTE;
  dataToSave.brewSetpoint = state.brewSetpoint;
  dataToSave.steamSetpoint = state.steamSetpoint;
  dataToSave.rtdOffset = state.rtdOffset;
  dataToSave.pid = pid;

  // EEPROM.put() writes structured data safely
  EEPROM.put(EEPROM_START_ADDR, dataToSave);
}

// inefficiently does a save-read-save loop, but i'm lazy
void reset(MachineState& state, PIDGains& pid) {
  SettingsEEPROM fakeData;
  fakeData.magic = EEPROM_MAGIC_BYTE + 1;
  EEPROM.put(EEPROM_START_ADDR, fakeData);
  loadSettings(state, pid);
}