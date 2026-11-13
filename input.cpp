#include "input.h"
#include "config.h"

volatile bool Input::encoderActivityFlag = false;

Input::Input() 
  : lastPosition(0),
    buttonPressStart(0),
    buttonPressed(false),
    longClickTriggered(false) {}

void Input::handleInterrupt() {
  encoderActivityFlag = true;
}

bool Input::begin(uint8_t addr) {
  if (!ss.begin(addr)) {
    return false;
  }

  ss.pinMode(BUTTON_PIN, INPUT_PULLUP);
  ss.setGPIOInterrupts(1UL << BUTTON_PIN, true);
  ss.enableEncoderInterrupt();

  pinMode(PIN_ENCODER_INT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCODER_INT), Input::handleInterrupt, FALLING);

  lastPosition = ss.getEncoderPosition();
  return true;
}

bool Input::poll(int &delta, bool &shortClick, bool &longClick) {
  delta = 0;
  shortClick = false;
  longClick = false;

  // If the interrupt flag hasn't tripped AND the button isn't actively being held down, skip checking I2C
  if (!encoderActivityFlag && !buttonPressed) {
    return false;
  }

  // Clear flag safely
  encoderActivityFlag = false;
  bool interacted = false;

  // 1. Read Rotation Delta over I2C
  int32_t currentPosition = ss.getEncoderPosition();
  int32_t diff = currentPosition - lastPosition;
  
  if (diff != 0) {
    delta = (int)diff;
    lastPosition = currentPosition;
    interacted = true;
  }

  // 2. Read Pushbutton State (Active Low: 0 = pressed)
  bool isPressedNow = (ss.digitalRead(BUTTON_PIN) == LOW);
  unsigned long now = millis();
  if (isPressedNow && !buttonPressed) {
    buttonPressed = true;
    buttonPressStart = now;
    longClickTriggered = false;
    interacted = true;
  }
  else if (isPressedNow && buttonPressed) {
    if (!longClickTriggered && (now - buttonPressStart >= LONG_PRESS_MS)) {
      longClick = true;
      longClickTriggered = true;
      interacted = true;
    }
  }
  else if (!isPressedNow && buttonPressed) {
    buttonPressed = false;
    if (!longClickTriggered) {
      shortClick = true;
      interacted = true;
    }
    longClickTriggered = false;
  }

  return interacted;
}
