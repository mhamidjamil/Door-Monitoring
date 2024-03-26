// Battery will charge when ac input is present
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
#include <ESP32Servo.h>
#include <WiFi.h>

#endif

#include <WiFiClient.h>

Servo servo;

void setup() {
  Serial.begin(115200);
  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

#if defined(ESP32)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
#elif defined(ESP8266)
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

#endif

  servo.attach(SERVO_PIN);
}

void loop() {}