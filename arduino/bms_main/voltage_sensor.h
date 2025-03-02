#ifndef VOLTAGE_SENSOR_H
#define VOLTAGE_SENSOR_H

#include "config.h"

float readVoltage(
  int pin,
  float r1 = 100000.0f,
  float r2 = 10000.0f,
  int sampleCount = SAMPLE_COUNT,
  int delayMs = DELAY_MS,
  float vRef = V_REF,
  int adcResolution = ADC_RESOLUTION) {

  long totalValue = 0;
  for (int i = 0; i < sampleCount; i++) {
    totalValue += analogRead(pin);
    delay(delayMs);
  }

  float averageValue = static_cast<float>(totalValue) / sampleCount;
  float vOut = averageValue * vRef / (adcResolution - 1);
  float vIn = vOut * (r1 + r2) / r2;

  if (DEBUG) {
    Serial.println(F("Debug: Voltage Sensor"));
    Serial.print(F("Average ADC Value: "));
    Serial.println(averageValue);
    Serial.print(F("Voltage Out (Measured): "));
    Serial.println(vOut, 4);
    Serial.print(F("Calculated Input Voltage: "));
    Serial.println(vIn, 4);
    Serial.println(F("--------------------"));
  }

  return vIn;
}

#endif
