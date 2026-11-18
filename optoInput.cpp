#include "optoInput.h"

OptoInput::OptoInput() {}

bool OptoInput::begin() {
  // Configure pins as inputs with internal pullups 
  // (assuming optocouplers pull the line LOW when active)
  pinMode(PIN_OPTO_PUMP, INPUT_PULLUP);
  pinMode(PIN_OPTO_STEAM, INPUT_PULLUP);
  return true;
}

void OptoInput::update(MachineState& state) {

  bool currentPumpState = (digitalRead(PIN_OPTO_PUMP) == LOW);

  // NB: static variable retains its value between function calls
  static bool previousPumpState = false;
  // detect the rising edge to capture start time exactly once
  if (currentPumpState && !previousPumpState) {
    state.shotStartTime = millis();
  }

  // Update state booleans
  state.isPumpOn = currentPumpState;
  state.isSteamMode = (digitalRead(PIN_OPTO_STEAM) == LOW);

  // Save current state for the next cycle's comparison
  previousPumpState = currentPumpState;
}