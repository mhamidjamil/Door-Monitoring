// in this code i call blynk api manually

#include "arduino_secrets.h"

#include <HTTPClient.h>
#include <WiFi.h>

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
    } else {
      Serial.println("ERROR: Unknown");
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

void updateBlynkState(int state) { updateBlynkState(1, state); }

String updateBlynkState(int pin, int state) {

  if (!isDeviceConnected()) {
    println("Device not connected to internet");
    return "Error: Device not connected to internet";
  }

  if (getPinState(pin) == state) {
    println("Pin is already in state:  " + String(state));
    return "Warning: Pin is already in state:  " + String(state);
  }

  String apiStatus = "";
  String apiUrl =
      blynkServer + "update?" + blynkParam + "&v" + pin + "=" + String(state);
  HTTPClient http;

  Serial.print("Sending API request to Blynk: ");
  Serial.println(apiUrl);

  http.begin(apiUrl);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      Serial.println("API request successful");
      apiStatus = "API request successful";
    } else {
      Serial.print("Error in API request. HTTP error code: ");
      Serial.println(httpCode);
      apiStatus = "Error in API request. HTTP error code: " + String(httpCode);
    }
  } else {
    Serial.println("Error in API request. Connection failed");
    apiStatus = "Error in API request. Connection failed";
  }

  http.end();
  return apiStatus;
}

int getPinState(int pin) {
  String apiUrl = blynkServer + "get?" + blynkParam + "&v" + String(pin);
  HTTPClient http;

  Serial.print("Sending API request to Blynk: ");
  Serial.println(apiUrl);

  http.begin(apiUrl);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      int state = response.toInt();
      Serial.print("Current state of pin ");
      Serial.print(pin);
      Serial.print(": ");
      Serial.println(state);
      return state;
    } else {
      Serial.print("Error in API request. HTTP error code: ");
      Serial.println(httpCode);
    }
  } else {
    Serial.println("Error in API request. Connection failed");
  }

  http.end();
  return -1; // Return -1 if there was an error
}

bool blynk_device_online() { return true; }

bool isDeviceConnected() {
  HTTPClient http;

  // Construct the API URL
  String apiUrl = blynkServer + "isHardwareConnected?" + blynkParam;

  // Send HTTP GET request
  http.begin(apiUrl);
  int httpCode = http.GET();

  // Check if request was successful
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    return response.equals("true"); // Convert response to boolean
  } else {
    Serial.print("Error in API request. HTTP error code: ");
    Serial.println(httpCode);
    return false; // Return false in case of error
  }

  http.end();
}

void println(String str) { Serial.println(str); }

void print(String str) { Serial.print(str); }