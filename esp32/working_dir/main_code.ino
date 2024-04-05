// Main code for home

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

#include "Cspiffs.h"
#include "c_led.h"

Cspiffs cspi;
c_led led;

unsigned int recheck_internet_connectivity = 300; // in seconds

//$ BLE stuff:
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

// #include "c_blynk.h"
// c_blynk blynk;
// no need for blynk for this code as it work on api calls

void BLE_inputManager(String input);

unsigned int BLE_last_receive_time = 0;
String BLE_Input = "";
String BLE_Output = "";

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
std::string receivedData = "";

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) { deviceConnected = true; }

  void onDisconnect(BLEServer *pServer) { deviceConnected = false; }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
      receivedData = value;
      Serial.print("Received from BLE: ");
      Serial.println(receivedData.c_str());
      BLE_inputManager(String(receivedData.c_str()));

      // Print the received data back to the BLE connection
      receivedData = BLE_Output.c_str();
      pCharacteristic->setValue("BLE: " + receivedData);
      pCharacteristic->notify();
    }
  }
};

void BLE_inputManager(String input) {
  bool time_out =
      BLE_Input != "" && (10 < ((millis() / 1000) - BLE_last_receive_time));
  if (input.indexOf("#") != -1 || time_out) {
    // means string is now fetched completely
    if (!isIn(input, "_nil_"))
      BLE_Input += input.substring(0, input.indexOf("#"));
    println("Executing (BLE input): {" + BLE_Input + "}");
    inputManager(removeNewline(BLE_Input));
    BLE_Input = "";
    BLE_last_receive_time = 0;
  } else if (input.length() > 0 && !isIn(input, "_nil_")) {
    if (BLE_Input == "")
      BLE_last_receive_time = (millis() / 1000);
    BLE_Input += input;
    if (BLE_Input.length() > 50) {
      println("BLE input is getting to large , here is the current string (" +
              BLE_Input + ") flushing it...");
      BLE_Input = "";
    }
  } else {
    if (!isIn(input, "_nil_"))
      println("!!!_______ unexpected error ______!!!");
    // rise("Unexpected flow in BLE input manager", "324");
  }
}

const int led_pin = 2;
const int white_led = 5;

unsigned int door_last_open_on = 0;
int close_door_in = 5; // in seconds
bool door_state = false;
int door_pin = led_pin;
bool BYPASS_PREVIOUS_DOOR_STATE = false;
bool ALLOW_DOOR_OPENING = true;
unsigned int blinker_last_on = 0;
int blinker_delay = 15;
int blinker_for = 500;
bool TRY_FILE_WIFI_CREDS = false;

bool isConnectedToWifi = false;
bool hasInternet = false;

void setup() {
  Serial.begin(115200);
  initBLE();

  pinMode(led_pin, OUTPUT);
  pinMode(white_led, OUTPUT);

  syncSPIFFS();
  connectToWifiAndBlynk(); // Attempt initial connection
}

void loop() {
  Blynk.run();

  if (BLE_Input != "")
    BLE_inputManager("_nil_");
  if (deviceConnected) {
    if (!oldDeviceConnected) {
      Serial.println("Connected to device");
      oldDeviceConnected = true;
    }
  } else {
    if (oldDeviceConnected) {
      Serial.println("Disconnected from device");
      delay(500);
      pServer->startAdvertising(); // restart advertising
      Serial.println("Restart advertising");
      oldDeviceConnected = false;
    }
  }

  if (Serial.available() > 0) {
    String receivedString = Serial.readStringUntil('\n');
    inputManager(receivedString);
  }

  static unsigned long lastCheckTime = 0;
  unsigned long currentTime = millis() / 1000;
  if (currentTime - lastCheckTime >= recheck_internet_connectivity) {
    lastCheckTime = currentTime;
    checkInternetConnectivity();
  }

  manageBackGroundJobs();
}

void inputManager(String command) {
  Serial.println("working on: " + command);
  if (isIn(command, "led on")) {
    led.led_on();
  } else if (isIn(command, "led off")) {
    led.led_off();
  } else if (isIn(command, "led2 on")) {
    led.led_on(white_led);
  } else if (isIn(command, "led2 off")) {
    led.led_off(white_led);
  } else if (isIn(command, "value of:")) {
    String varName =
        cspi.getVariableName(command.substring(command.indexOf(":")), ":");
    String targetValue = cspi.getFileVariableValue(varName);
    println("\t\tValue of: " + varName + " is: [" + targetValue + "]");
    if (command.indexOf(" to ") != -1) {
      String newValue = command.substring(command.indexOf(" to ") + 4, -1);
      println("Updating value to: [" + newValue + "]");
      cspi.updateSPIFFS(varName, newValue);
    }
    syncSPIFFS();
  } else if (isIn(command, "readSPIFFS")) {
    println("Reading SPI data\n\n");
    cspi.readSPIFFS();
    println("<- END \n\n");
  } else {
    Serial.println("Nothing executed");
  }
}

void connectToWifiAndBlynk() {
  Serial.println("Connecting to Wi-Fi...");

  bool connected_with_wifi = false;
  if (TRY_FILE_WIFI_CREDS) {
    String ssidString = cspi.getFileVariableValue("new_wifi_name");
    char file_ssid[ssidString.length() + 1]; // Add 1 for null terminator
    ssidString.toCharArray(file_ssid, sizeof(file_ssid));

    String passwordString = cspi.getFileVariableValue("new_wifi_password");
    char
        file_password[passwordString.length() + 1]; // Add 1 for null terminator
    passwordString.toCharArray(file_password, sizeof(file_password));
    println("Connecting to file ssid:" + String(file_ssid) +
            " password: " + String(file_password));
    connected_with_wifi = validateWIFICreds(file_ssid, file_password);
    if (connected_with_wifi)
      Blynk.begin(BLYNK_AUTH_TOKEN, file_ssid, file_password);
  }

  if (!connected_with_wifi) {
    println("Connecting to ssid: " + String(ssid));
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  }

  println("Trying to connect to blynk");
  if (Blynk.connect()) {
    Serial.println("Connected to Blynk");
  } else {
    Serial.println("Failed to connect to Blynk");
    if (TRY_FILE_WIFI_CREDS) {
      TRY_FILE_WIFI_CREDS = false;
      connectToWifiAndBlynk(); // try again
    }
  }
}

void checkInternetConnectivity() {
  Serial.println("Checking internet connectivity...");
  WiFiClient client;
  bool isConnected = false;

  for (int i = 0; i < 3; i++) {
    if (client.connect("www.google.com", 80)) {
      isConnected = true;
      break;
    } else {
      delay(50);
    }
  }

  if (isConnected) {
    Serial.println("Internet connected");
    hasInternet = true;
    client.stop();
    // digitalWrite(led_pin, HIGH); // Turn on LED if internet is not connected
  } else {
    Serial.println("Internet not connected");
    hasInternet = false;
    client.stop();
    // digitalWrite(led_pin, LOW); // Turn off LED if internet is connected
  }
}

BLYNK_WRITE(V1) {
  int ledValue = param.asInt();
  if (ledValue == 1 || ledValue == HIGH) {
    door_last_open_on = getSeconds();
    door_state = true;
    println("Door state change, time stamp: " + door_last_open_on);
  } else {
    println("State: " + ledValue);
  }
  if (!ALLOW_DOOR_OPENING) {
    println("Door functionality is disabled please run "
            "[value of:ALLOW_DOOR_OPENING to 1] to enable");
    return;
  }
#if defined(ESP32)
  digitalWrite(led_pin, ledValue ? HIGH : LOW);
#elif defined(ESP8266)
  analogWrite(led_pin, ledValue ? 1023 : 0);
#endif
}

BLYNK_WRITE(V5) {
  int ledValue = param.asInt();
  digitalWrite(white_led, ledValue ? HIGH : LOW);
}

void println(String str) { Serial.println(str); }

void print(String str) { Serial.print(str); }

bool isIn(String mainString, String toFind) {
  return mainString.indexOf(toFind) != -1;
}

void initBLE() {
  BLEDevice::init("ESP32 Servo");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Generate unique UUIDs for service and characteristic
  BLEUUID serviceUUID(
      "1a8e61ca-82c3-4d8f-b635-2424a4c8cc72"); // Custom service UUID
  BLEUUID characteristicUUID(
      "67dbcc1b-8aa9-45b6-af99-fa06f9fd1a7c"); // Custom characteristic UUID

  BLEService *pService = pServer->createService(serviceUUID);
  pCharacteristic = pService->createCharacteristic(
      characteristicUUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(pService->getUUID());
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // sets the minimum advertising interval
  BLEDevice::startAdvertising();

  Serial.println("Waiting for Bluetooth connection...");
}

String removeNewline(String str) {
  str.replace("\n", " ");
  return str;
}

void createOwnNetwork() {
  // Set up your own network
  // Configure the SoftAP (Access Point)
  const char *ap_ssid = SELF_SSID;
  const char *ap_password = SELF_PASSWORD;

  // Create the access point
  WiFi.softAP(ap_ssid, ap_password);

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Own network created");
}

void manageBackGroundJobs() {
  closeDoorIfNot();
  blinker();
  delay(50);
}

void closeDoorIfNot() {
  if (((getSeconds() - door_last_open_on) > close_door_in) &&
      (door_state || BYPASS_PREVIOUS_DOOR_STATE)) {
    println("Auto closing door");
    digitalWrite(door_pin, LOW);
    door_state = false;

    // FIXME: will be updated when use servo motor
  }
}

void blinker() {
  if ((getSeconds() - blinker_last_on) > blinker_delay && blinker_delay != -1) {
    led.change_state_for(white_led, 1, blinker_for);
    // println("Blinker executed");
    blinker_last_on = getSeconds();
  }
}

unsigned int getSeconds() { return (millis() / 1000); }

void syncSPIFFS() {
  close_door_in = cspi.getFileVariableValue("close_door_in", true).toInt();
  BYPASS_PREVIOUS_DOOR_STATE =
      cspi.getFileVariableValue("BYPASS_PREVIOUS_DOOR_STATE", true).toInt() == 1
          ? true
          : false;
  blinker_delay = cspi.getFileVariableValue("blinker_delay", true).toInt();
  blinker_for = cspi.getFileVariableValue("blinker_for", true).toInt();
  ALLOW_DOOR_OPENING =
      cspi.getFileVariableValue("ALLOW_DOOR_OPENING", true).toInt() == 1
          ? true
          : false;
  TRY_FILE_WIFI_CREDS =
      cspi.getFileVariableValue("TRY_FILE_WIFI_CREDS", true).toInt() == 1
          ? true
          : false;
  recheck_internet_connectivity =
      cspi.getFileVariableValue("recheck_internet_connectivity", true).toInt();
}

void log(String msg) {
  print("\n\t@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\t");
  println(msg);
  println("\n\t@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
}

bool validateWIFICreds(char ssid[], char pass[]) {
  println("checking if wifi is connectable...");
  WiFi.begin(ssid, pass);
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (i > 10) {
      println("\n!!!---Timeout: Unable to connect to WiFi---!!!");
      return false;
      break;
    }
    delay(500);
    print(".");
    i++;
  }
  if (i < 10) {
    println("Connected.\n Disconnecting...");
    WiFi.disconnect(true);
    return true;
  }
}