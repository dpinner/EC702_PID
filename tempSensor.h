#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <Adafruit_MAX31865.h>
#include "config.h"

class TempSensor {
public:
  TempSensor();
  bool begin();
  float readTemperature(MachineState &state); // Returns temp and updates fault code
  bool updateTemperature(float currentTemp, MachineState &state); // updates offset value; NOTE: doesn't save to EEPROM

private:
  Adafruit_MAX31865 max31865;
};

#endif