#include "c_blynk.h"
#include <HTTPClient.h>

c_blynk::c_blynk() {
  Serial.begin(115200);
  println("\t\t\t###___Constructor___###");
}

String c_blynk::updateBlynkState(int pin, int state, bool forceUpdate) {

  if (!isDeviceConnected()) {
    println("Device not connected to internet");
    return "Error: Device not connected to internet";
  }

  if (!forceUpdate && getPinState(pin) == state) {
    println("Pin is already in state:  " + String(state));
    return "Warning: Pin is already in state:  " + String(state);
  }

  String apiStatus = "";
  String apiUrl =
      blynkServer + "update?" + blynkParam + "&v" + pin + "=" + String(state);
  HTTPClient http;

  Serial.print("Sending API request to Blynk: ");
  Serial.println(apiUrl);

  http.setTimeout(5000);
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

String c_blynk::updateBlynkState(int pin, int state) {
  return updateBlynkState(pin, state, false);
}

int c_blynk::getPinState(int pin) {
  String apiUrl = blynkServer + "get?" + blynkParam + "&v" + String(pin);
  HTTPClient http;

  Serial.print("Sending API request to Blynk: ");
  Serial.println(apiUrl);

  http.setTimeout(5000);
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

bool c_blynk::isDeviceConnected() {
  HTTPClient http;

  // Construct the API URL
  String apiUrl = blynkServer + "isHardwareConnected?" + blynkParam;

  // Send HTTP GET request
  http.setTimeout(5000);
  http.begin(apiUrl);
  int httpCode = http.GET();

  // Check if request was successful
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    println("Connection status: " + response);
    return response.equals("true"); // Convert response to boolean
  } else {
    Serial.print("Error in API request. HTTP error code: ");
    Serial.println(httpCode);
    return false; // Return false in case of error
  }

  http.end();
}

void c_blynk::println(String tempStr) { Serial.println(tempStr); }

void c_blynk::print(String tempStr) { Serial.print(tempStr); }
