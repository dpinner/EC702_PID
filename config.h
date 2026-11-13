#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <EEPROM.h>

// Hardware Pin Definitions (Arduino Nano R4)
#define PIN_ENCODER_INT   2
#define PIN_SSR_HEATER    3
#define PIN_OPTO_STEAM    7   // Optocoupler for steam mode switch
#define PIN_OPTO_PUMP     8   // Optocoupler for brewing / pump status

// MAX31865 SPI Pins
#define PIN_MAX_CS        10

// RTD Reference Resistor Constants
#define RREF              430.0f
#define RNOMINAL          100.0f

// Default Settings
#define DEFAULT_BREW_SETPOINT   93.0f
#define DEFAULT_STEAM_SETPOINT 125.0f
#define DEFAULT_TEMP_OFFSET      0.0f
#define DEFAULT_KP               2.0f
#define DEFAULT_KI               0.5f
#define DEFAULT_KD               0.1f
#define DEFAULT_BREW_FF         35.0f
#define DEFAULT_STEAM_FF        15.0f
#define MAX_SAFE_TEMP          145.0f

// EEPROM Configuration Constants
#define EEPROM_MAGIC_BYTE     0x42
#define EEPROM_START_ADDR     0
#define TELEMETRY_INTERVAL_MS 200

struct PIDGains {
  float brewKp;
  float steamKp;
  float brewKi;
  float steamKi;
  float brewKd;
  float steamKd;
  float brewFF;
  float steamFF;
};

// data saved to persistent EEPROM memory
struct SettingsEEPROM {
  uint8_t magic;
  float brewSetpoint;
  float steamSetpoint;
  float rtdOffset;
  PIDGains pid;
};

struct MachineState {
  float currentTemp;
  float brewSetpoint;
  float steamSetpoint;
  float rtdOffset;
  bool isSteamMode;
  bool isPumpOn;
  uint8_t sensorFaultCode;  // Stores the MAX31865 fault register value (0 = no fault)
  unsigned long shotStartTime;
  unsigned long shotDurationMillis;
};

void loadSettings(MachineState& state, PIDGains& pid);
void saveSettings(const MachineState& state, const PIDGains& pid);
void reset(MachineState& state, PIDGains& pid);

#endif