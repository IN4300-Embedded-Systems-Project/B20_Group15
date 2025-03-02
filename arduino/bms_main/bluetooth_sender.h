#ifndef BLUETOOTH_SENDER_H
#define BLUETOOTH_SENDER_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class BluetoothDataSender {
private:
  SoftwareSerial* BTSerial;

public:
  BluetoothDataSender(int rxPin, int txPin) {
    BTSerial = new SoftwareSerial(rxPin, txPin);
  }

  void begin(long baudRate) {
    BTSerial->begin(baudRate);
  }

  template<typename... Args>
  String createDataString(Args... args) {
    String result = "";
    int dummy[sizeof...(args)] = { (result += String(args) + ",", 0)... };
    if (result.length() > 0) {
      result.remove(result.length() - 1);  // Remove the last comma
    }
    return result;
  }

  void sendDataBT(String dataString) {
    BTSerial->println(dataString);

    if (DEBUG) {
      Serial.print("Debug: Sent data: ");
      Serial.println(dataString);
    }
  }

  template<typename... Args>
  void sendToApp(Args... args) {
    sendDataBT(createDataString(args...));
  }
};

#endif