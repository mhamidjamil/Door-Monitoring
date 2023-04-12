# Blynk ESP32 Door Monitoring System

This is a simple door monitoring system using an ESP32 and Blynk app.

## Requirements

- Arduino IDE
- ESP32 or ESP8266 board support for Arduino IDE
- Blynk app installed on your smartphone

## Hardware

- ESP32/ESP8266 board
- Magnetic door switch
- Servo motor
- LEDs
- Resistors
- Wires
- Breadboard

## Installation

1. Install the ESP32 board support for Arduino IDE. Follow [these instructions](https://github.com/espressif/arduino-esp32#installation-instructions).
2. Install the Blynk library for Arduino IDE. In the Arduino IDE, go to Sketch > Include Library > Manage Libraries, then search for "Blynk" and install it.
3. Download or clone this repository to your computer.
4. Open the `esp32-door-monitoring-system.ino` file in Arduino IDE.
5. Modify the following lines with your Wi-Fi and Blynk credentials:

char ssid[] = "your_wifi_ssid";
char pass[] = "your_wifi_password";
#define BLYNK_AUTH_TOKEN "your_blynk_auth_token"

6. Connect the hardware as shown in the diagram below.
7. Upload the code to your ESP32 board.
8. Open the Blynk app on your smartphone and create a new project.
9. Add the following widgets to your project:

- Slider widget on V3
- Button widget on V7

10. Set the properties of the widgets as follows:

- Slider widget on V3:
  - Output: Virtual Pin V3
  - Min value: 0
  - Max value: 180
- Button widget on V7:
  - Output: Virtual Pin V7
  - Mode: Switch
  - OFF value: 0
  - ON value: 1

11. Run the project and enjoy monitoring your door!
