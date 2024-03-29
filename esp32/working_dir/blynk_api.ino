// in this code i call blynk api manually

#include "arduino_secrets.h"

#include <HTTPClient.h>
#include <WiFi.h>

#include "c_blynk.h"
c_blynk blynk;
// need this library as it will call the api

const char *ssid = MY_SSID;
const char *password = MY_PASSWORD;

String blynkServer = "https://sgp1.blynk.cloud/external/api/";
String blynkParam = "token=" + String(MY_AUTH_TOKEN);

const int LED_PIN = 2; // Replace with your LED pin number

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  connectToWiFi();
}

void loop() {
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == '1') {
      updateBlynkState(1); // Turn on LED
    } else if (input == '0') {
      updateBlynkState(0); // Turn off LED
    }
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected!");
}

void updateBlynkState(int state) { blynk.updateBlynkState(1, state); }