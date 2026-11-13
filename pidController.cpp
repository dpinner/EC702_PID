#include "pidController.h"

PIDController::PIDController()
  : lastComputeTime(0),
    windowStartTime(0),
    iTerm(0.0f),
    lastInput(0.0f),
    currentOutput(0.0f),
    initialized(false) {}

void PIDController::begin() {
  pinMode(PIN_SSR_HEATER, OUTPUT);
  digitalWrite(PIN_SSR_HEATER, LOW);
  windowStartTime = millis();
  lastComputeTime = millis();
  initialized = false;
}

// Largely follows http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/
float PIDController::compute(const MachineState& state, const PIDGains& gains) {
  // Safety override: if sensor fault code is active or we exceed safe temperatures, kill heater immediately
  if (state.sensorFaultCode != 0 || state.currentTemp > MAX_SAFE_TEMP) {
    digitalWrite(PIN_SSR_HEATER, LOW);
    iTerm = 0.0f;
    currentOutput = 0.0f;
    initialized = false; // Reset so we re-initialize safely when fault clears
    return currentOutput;
  }

  unsigned long now = millis();
  // unsigned long is modular, and so rollover-safe
  unsigned long timeChange = now - lastComputeTime;

  // 1. Stable Sample-Time Pacing (Run PID math every 100ms)
  if (timeChange >= sampleTimeMs) {
    float dt = (float)timeChange / 1000.0f;
    lastComputeTime = now;

    float setpoint = state.isSteamMode ? state.steamSetpoint : state.brewSetpoint;
    float kp = state.isSteamMode ? gains.steamKp : gains.brewKp;
    float ki = state.isSteamMode ? gains.steamKi : gains.brewKi;
    float kd = state.isSteamMode ? gains.steamKd : gains.brewKd;

    float error = setpoint - state.currentTemp;

    float pTerm = kp * error;

    // include ki in integral to support on-the-fly tuning
    iTerm += ki * error * dt;
    // "anti-windup" -- don't allow integral term to grow without bound
    iTerm = constrain(iTerm, outMin, outMax);

    // derivative on measurement (eliminates derivative kick on setpoint change)
    float dInput = 0.0f; // prevent initial derivative spikes
    if (!initialized) {
      initialized = true;
    } else {
      dInput = (state.currentTemp - lastInput) / dt;
    }
    float dTerm = -kd * dInput;

    // Feed-Forward Control
    float feedForward = 0.0f;
    if (state.isPumpOn) {
      feedForward = gains.brewFF; // offsets cold water in-rush
    } else if (state.isSteamMode) {
      feedForward = gains.steamFF; // baseline maintenance power for steam
    }

    // save input for next derivative cycle
    lastInput = state.currentTemp;

    float output = pTerm + iTerm + dTerm + feedForward;
    currentOutput = constrain(output, outMin, outMax);
  }

  // 2. Time-Proportioning Control (Slow PWM for AC Solid State Relay)
  // Services every loop iteration for precise duty cycle timing
  unsigned long currentMillis = millis();
  if (currentMillis - windowStartTime >= windowSizeMs) {
    windowStartTime += windowSizeMs; // Roll window forward without drift
  }

  unsigned long onTime = (windowSizeMs * (unsigned long)currentOutput) / 100;

  if (currentMillis - windowStartTime < onTime) {
    digitalWrite(PIN_SSR_HEATER, HIGH);
  } else {
    digitalWrite(PIN_SSR_HEATER, LOW);
  }

  return currentOutput;
}