#include <Wire.h>
#include "config.h"
#include "display.h"
#include "input.h"
#include "tempSensor.h"

MachineState state = {
  .currentTemp = 0.0f,
  .brewSetpoint = DEFAULT_BREW_SETPOINT,
  .steamSetpoint = DEFAULT_STEAM_SETPOINT,
  .rtdOffset = 0.0f,
  .isSteamMode = false,
  .isPumpOn = false,
  .sensorFaultCode = 0x00,
  .shotStartTime = 0,
  .shotDurationMillis = 0
};

PIDGains gains = {
  .brewKp = 2.0f, 
  .steamKp = 2.0f, 
  .brewKi = 0.5f, 
  .steamKi = 0.5f, 
  .brewKd = 0.1f,
  .steamKd = 0.1f
};

Display ui;
TempSensor tempSensor;
Input input;

unsigned long lastTelemetryTime = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  loadSettings(state, gains);
  while (!Serial && millis() < 2000); // wait up to 2s for USB connection

  if (!tempSensor.begin()) {
    Serial.println("FATAL: RTD Initialization Failed!");
  }
  if (!ui.begin()) {
    Serial.println("WARNING: LCD Initialization Failed!");
  }
  if (!input.begin()) {
    Serial.println("WARNING: Encoder Initialization Failed!");
  }
}

void loop() {
  // 1. Read Sensor & Update Temperature / Fault State
  state.currentTemp = tempSensor.readTemperature(state);
  // 2. Read Machine Inputs
  // TODO
  // 3. Read User Inputs
  int delta = 0;
  bool shortClick = false;
  bool longClick = false;
  if (input.poll(delta, shortClick, longClick)) {
    ui.handleInput(delta, shortClick, longClick, state, gains);
  }
  // 4. Update Display
  ui.update(state, gains);
  // 5. USB Telemetry Streaming (Non-blocking)
  if (Serial) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastTelemetryTime >= TELEMETRY_INTERVAL_MS) {
      lastTelemetryTime = currentMillis;

      float activeSetpoint = state.isSteamMode ? state.steamSetpoint : state.brewSetpoint;
      
      // Output format optimized for Arduino Serial Plotter: Label:Value, Label:Value
      Serial.print("Temp:");
      Serial.print(state.currentTemp, 2);
      Serial.print(", Setpoint:");
      Serial.println(activeSetpoint, 2);
    }
  }
}
