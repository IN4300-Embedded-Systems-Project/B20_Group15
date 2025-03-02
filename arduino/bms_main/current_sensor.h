#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include "config.h"

float readCurrentSensor(
  int pin,
  float voltageOffset = 2.481,  // Adjust this value to make the reading 0.0 when no current is flowing.
  float sensitivity = 0.100,    // Adjust this depending on your sensor (5A - 185, 20A - 100, 30A - 66).
  int sampleCount = SAMPLE_COUNT,
  int delayMs = DELAY_MS,
  float referenceVoltage = V_REF,
  float adcResolution = ADC_RESOLUTION) {

  float Samples = 0.0;
  for (int x = 0; x < sampleCount; x++) {
    Samples += analogRead(pin);
    delay(delayMs);
  }

  float AvgAcs = Samples / sampleCount;
  float AcsValueF = (voltageOffset - AvgAcs * referenceVoltage / (adcResolution - 1)) / sensitivity;

  if (DEBUG) {
    Serial.println(F("Debug: Current Sensor"));
    Serial.print(F("Average ADC Value: "));
    Serial.println(AvgAcs);
    Serial.print(F("Calculated Current: "));
    Serial.println(AcsValueF, 4);
    Serial.println(F("--------------------"));
  }

  return AcsValueF;
}

#endif
