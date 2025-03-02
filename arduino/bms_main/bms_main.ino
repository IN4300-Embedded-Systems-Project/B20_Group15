#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "current_sensor.h"
#include "voltage_sensor.h"
#include "temperature_sensor.h"
#include "bluetooth_sender.h"
#include "config.h"

#define CURRENT_SENSOR_PIN A0
#define VOLTAGE_SENSOR_PIN A1
#define TEMPERATURE_SENSOR_PIN A2

// Adjust these values based on your battery specifications
#define BATTERY_CAPACITY 3350.0      // mAh
#define BATTERY_NOMINAL_VOLTAGE 3.6  // V
#define BATTERY_MIN_VOLTAGE 2.5      // V
#define BATTERY_MAX_VOLTAGE 4.2      // V

LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust the address (0x27) if needed
BluetoothDataSender btSender(2, 3);

byte batteryIcon[8] = {
  0b01110,
  0b11011,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b10001,
  0b11111
};

byte heartIcon[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};

bool alternateScreen = false;

void setup() {
  Serial.begin(9600);
  btSender.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, batteryIcon);
  lcd.createChar(1, heartIcon);
}

float calculateBatteryHealth(float voltage, float current, float temperature) {
  // Simple battery health calculation (adjust as needed)
  float health = 100.0;

  if (voltage < BATTERY_NOMINAL_VOLTAGE) {
    health -= 10.0 * (BATTERY_NOMINAL_VOLTAGE - voltage) / (BATTERY_NOMINAL_VOLTAGE - BATTERY_MIN_VOLTAGE);
  }

  if (temperature > 40.0 || temperature < 0.0) {
    health -= 5.0;
  }

  if (abs(current) > 2.0) {  // Adjusted for typical Li-ion battery
    health -= 5.0;
  }

  return constrain(health, 0.0, 100.0);
}

float calculateBatteryPercentage(float voltage) {
  return constrain(map(voltage * 100, BATTERY_MIN_VOLTAGE * 100, BATTERY_MAX_VOLTAGE * 100, 0, 100), 0, 100);
}

void displayData(float current, float voltage, float temperature, float batteryHealth, float batteryPercentage) {
  lcd.clear();

  if (alternateScreen) {
    lcd.setCursor(0, 0);
    lcd.print("I:");
    lcd.print(current, 2);
    lcd.print("A V:");
    lcd.print(voltage, 2);
    lcd.print("V");

    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C");
  } else {
    lcd.setCursor(0, 0);
    lcd.write(byte(0));  // Battery icon
    lcd.print(" Bat: ");
    lcd.print(batteryPercentage, 0);
    lcd.print("%");

    lcd.setCursor(0, 1);
    lcd.write(byte(1));  // Heart icon
    lcd.print(" Health: ");
    lcd.print(batteryHealth, 0);
    lcd.print("%");
  }

  alternateScreen = !alternateScreen;
}

void loop() {
  float current = readCurrentSensor(CURRENT_SENSOR_PIN);
  float voltage = readVoltage(VOLTAGE_SENSOR_PIN);
  float temperature = readTemperature(TEMPERATURE_SENSOR_PIN);

  float batteryHealth = calculateBatteryHealth(voltage, current, temperature);
  float batteryPercentage = calculateBatteryPercentage(voltage);

  displayData(current, voltage, temperature, batteryHealth, batteryPercentage);
  btSender.sendToApp(current, voltage, temperature, batteryHealth, batteryPercentage);

  // Print to Serial for debugging
  Serial.print("Current: ");
  Serial.print(current);
  Serial.print("A, Voltage: ");
  Serial.print(voltage);
  Serial.print("V, Temp: ");
  Serial.print(temperature);
  Serial.print("C, Health: ");
  Serial.print(batteryHealth);
  Serial.print("%, Charge: ");
  Serial.print(batteryPercentage);
  Serial.println("%");

  delay(3000);  // Display each screen for 3 seconds
}
