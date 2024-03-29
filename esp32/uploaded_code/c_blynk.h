//$ last change: 29/March/24  01:05 AM

#ifndef C_BLYNK
#define C_BLYNK_H
#include "arduino_secrets.h"
#include <Arduino.h>

class c_blynk {

private:
  String blynkServer = "https://sgp1.blynk.cloud/external/api/";
  String blynkParam = "token=" + String(MY_AUTH_TOKEN);

public:
  c_blynk();

  String updateBlynkState(int pin, int state, bool forceUpdate);
  String updateBlynkState(int pin, int state);
  int getPinState(int pin);
  bool isDeviceConnected();

  void print(String tempStr);
  void println(String tempStr);
};

#endif
