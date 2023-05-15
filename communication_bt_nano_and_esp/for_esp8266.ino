#include <SoftwareSerial.h>

SoftwareSerial arduinoSerial(2, 3);  // RX, TX pins for communication with Arduino Nano

void setup() {
  Serial.begin(9600);  // Serial monitor
  arduinoSerial.begin(9600);  // Communication with Arduino Nano
}

void loop() {
  // Send known and unknown commands every 3 to 5 seconds
  if (millis() % 3000 == 0) {
    // Send known command
    arduinoSerial.println("#open!");
    delay(10);  // Delay to allow the message to be sent

    // Send unknown command
    arduinoSerial.println("#unknown!");
    delay(10);  // Delay to allow the message to be sent
  }

  // Check for responses from Arduino Nano
  if (arduinoSerial.available()) {
    String response = arduinoSerial.readStringUntil('\n');
    Serial.println(response);
  }
}
