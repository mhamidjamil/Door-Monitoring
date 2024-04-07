// Cspiffs.h
#ifndef C_LED
#define C_LED_H
#include <Arduino.h>

#define LED 6

class c_led {

private:
  int builtin_led = 2;

public:
  c_led();
  void led(int pinNumber, bool state);
  void led_on();
  void led_off();
  void led_on(int pin_number);
  void led_off(int pin_number);
  void change_state_for(int pin, int state, int time);
  void blink(int times, int delay_);
  void print(String tempStr);
  void println(String tempStr);                 // actually Serial.println()
  void println(String tempStr, bool condition); // only print if it has to be
};

#endif
