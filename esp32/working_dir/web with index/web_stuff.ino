#include "arduino_secrets.h"

#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <WiFiClient.h>

const char *ssid = MY_SSID;
const char *password = MY_PASSWORD;

WebServer server(420); // Create a WebServer object listening on port 420

// Define variables to store the values
String variable1 = "Default value 1";
String variable2 = "Default value 2";
String variable3 = "Default value 3";

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
  server.on("/getVariables", HTTP_GET, handleGetVariables);

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
void handleUpdate() {
  if (server.method() != HTTP_POST) {
    server.send(405, "text/plain", "Method Not Allowed");
    return;
  }

  String json = server.arg("plain");
  DynamicJsonDocument requestData(200);
  DeserializationError error = deserializeJson(requestData, json);

  if (error) {
    Serial.println("Failed to parse JSON");
    server.send(400, "text/plain", "Bad Request");
    return;
  }

  // Update the variables with the new values
  variable1 = requestData["variable1"].as<String>();
  variable2 = requestData["variable2"].as<String>();
  variable3 = requestData["variable3"].as<String>();

  Serial.println("Variables updated:");
  Serial.println("Variable 1: " + variable1);
  Serial.println("Variable 2: " + variable2);
  Serial.println("Variable 3: " + variable3);

  server.send(200, "application/json",
              "{\"status\": \"success\", \"message\": \"Variables updated "
              "successfully\"}");
}

// Handle GET request to retrieve variable values
void handleGetVariables() {
  // Create a JSON object to store the variable values
  StaticJsonDocument<200> responseData;
  responseData["variable1"] = variable1;
  responseData["variable2"] = variable2;
  responseData["variable3"] = variable3;

  // Serialize the JSON object to a string
  String responseJson;
  serializeJson(responseData, responseJson);

  // Send the JSON response to the client
  server.send(200, "application/json", responseJson);
}
