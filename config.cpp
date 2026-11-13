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
    pid.brewKp = 2.0f;
    pid.steamKp = 2.0f;
    pid.brewKi = 0.5f;
    pid.steamKi = 0.5f;
    pid.brewKd = 0.1f;
    pid.steamKd = 0.1f;
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