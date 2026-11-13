#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

class PIDController {
public:
  PIDController();
  void begin();
  float compute(const MachineState& state, const PIDGains& gains);

private:
  unsigned long lastComputeTime;
  unsigned long windowStartTime;
  const unsigned long windowSizeMs = 1000; // 1-second time-proportioning window
  const unsigned long sampleTimeMs = 100;  // 100ms PID math update interval

  float iTerm;         // Accumulated integral term
  float lastInput;     // For derivative on measurement
  float currentOutput; // Latest computed PID output (0 - 100 %)
  bool initialized;    // we'll use this to prevent a huge derivative kick on startup

  const float outMin = 0.0f;
  const float outMax = 100.0f;
};

#endif