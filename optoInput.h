#ifndef OPTO_INPUT_H
#define OPTO_INPUT_H

#include <Arduino.h>
#include "config.h"

class OptoInput {
public:
  OptoInput();
  bool begin();
  void update(MachineState& state);
};

#endif