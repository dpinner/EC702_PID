#include "tempSensor.h"

TempSensor::TempSensor() : max31865(PIN_MAX_CS) {}

bool TempSensor::begin() {
  if (!max31865.begin(MAX31865_3WIRE)) {
    return false;
  }
  return true;
}

float TempSensor::readTemperature(MachineState &state) {
  state.sensorFaultCode = max31865.readFault();
  if (state.sensorFaultCode) {
    max31865.clearFault();
    return -999.0f; // Sentinel error value
  }

  float rawTemp = max31865.temperature(RNOMINAL, RREF);
  return rawTemp + state.rtdOffset;
}