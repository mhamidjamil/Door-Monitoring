#include "arduino_secrets.h"

#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>

const char *ssid = MY_SSID;
const char *password = MY_PASSWORD;

WebServer server(420); // Create a WebServer object listening on port 420

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Failed to mount file system");
    return;
  }

  // Set up routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);

  // Start server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient(); // Handle client requests
}

// Handle root page request
void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  server.streamFile(file, "text/html"); // Stream HTML file to client
  file.close();
}

// Handle update request
// Handle update request
void handleUpdate() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String jsonString = server.arg("plain");
  Serial.println("Received JSON data:");
  Serial.println(jsonString);

  DynamicJsonDocument requestData(200);
  DeserializationError error = deserializeJson(requestData, jsonString);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  // Extract variables from JSON data
  String variable1 = requestData["variable1"].as<String>();
  String variable2 = requestData["variable2"].as<String>();
  String variable3 = requestData["variable3"].as<String>();

  // Print received data to Serial Monitor
  Serial.println("Variables received:");
  Serial.print("Variable 1: ");
  Serial.println(variable1);
  Serial.print("Variable 2: ");
  Serial.println(variable2);
  Serial.print("Variable 3: ");
  Serial.println(variable3);

  // Send response to client
  server.send(200, "application/json",
              "{\"status\": \"success\", \"message\": \"Variables updated "
              "successfully\"}");
}
