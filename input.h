#ifndef INPUT_H
#define INPUT_H

#include <Arduino.h>
#include <Adafruit_seesaw.h>  // For STEMMA QT Rotary Encoder

#define DEFAULT_ADDR  0x36
#define BUTTON_PIN    24
#define LONG_PRESS_MS 800

class Input {
public:
  Input();
  bool begin(uint8_t addr = DEFAULT_ADDR);
  bool poll(int &delta, bool &shortClick, bool &longClick);

  static void handleInterrupt();

private:
  Adafruit_seesaw ss;
  int32_t lastPosition;
  unsigned long buttonPressStart;
  bool buttonPressed;
  bool longClickTriggered;

  static volatile bool encoderActivityFlag;
};

#endif