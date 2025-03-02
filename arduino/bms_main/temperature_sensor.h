#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <math.h>
#include "config.h"


double Thermistor(int RawADC) {
  double Temp;
  Temp = log(10240000 / RawADC - 10000);
  Temp = 1 / (0.001129148 + (0.000234125 * Temp) + (0.0000000876741 * Temp * Temp * Temp));
  Temp = Temp - 273.15;  // This will Convert Kelvin to Celsius
  return Temp;
}

double readTemperature(
  int pin,
  int sampleCount = SAMPLE_COUNT,
  int delayMs = DELAY_MS) {

  long totalValue = 0;
  for (int i = 0; i < sampleCount; i++) {
    totalValue += analogRead(pin);
    delay(delayMs);
  }

  int averageADC = static_cast<int>(totalValue / sampleCount);
  int mappedADC = map(averageADC, 0, ADC_RESOLUTION - 1, ADC_RESOLUTION - 1, 0);
  double Temp = Thermistor(mappedADC);

  if (DEBUG) {
    Serial.println(F("Temperature Sensor Debug Information:"));
    Serial.print(F("Average ADC Value: "));
    Serial.println(averageADC);
    Serial.print(F("Mapped ADC Value: "));
    Serial.println(mappedADC);
    Serial.print(F("Calculated Temperature (°C): "));
    Serial.println(Temp, 2);
    Serial.println(F("--------------------"));
  }

  return Temp;
}

#endif