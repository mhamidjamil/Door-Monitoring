#include "arduino_secrets.h"

#define BLYNK_TEMPLATE_ID MY_ID
#define BLYNK_TEMPLATE_NAME MY_TEMPLATE_NAME
#define BLYNK_AUTH_TOKEN MY_AUTH_TOKEN

char ssid[] = MY_SSID;
char pass[] = MY_PASSWORD;

#define BLYNK_PRINT Serial

#if defined(ESP8266)
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#elif defined(ESP32)
#include <BlynkSimpleEsp32.h>
#include <WiFi.h>
#endif

#include <WiFiClient.h>

const int LED_PIN =
    2; // Replace 2 with the pin number where your built-in LED is connected

void setup() {
  Serial.begin(115200);

#if defined(ESP32)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(LED_PIN, OUTPUT);
#elif defined(ESP8266)
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  pinMode(LED_PIN, OUTPUT);
#endif
}

void loop() { Blynk.run(); }

BLYNK_WRITE(V1) {
  int ledValue = param.asInt();
#if defined(ESP32)
  digitalWrite(LED_PIN, ledValue ? HIGH : LOW);
#elif defined(ESP8266)
  analogWrite(LED_PIN, ledValue
                           ? 1023
                           : 0); // For ESP8266, use analogWrite for LED dimming
#endif
}
