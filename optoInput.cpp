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
  // Update state booleans based on pin readings
  // Note: Change == LOW to == HIGH if your specific opto circuit outputs active-high
  state.isPumpOn = (digitalRead(PIN_OPTO_PUMP) == LOW);
  state.isSteamMode = (digitalRead(PIN_OPTO_STEAM) == LOW);
}