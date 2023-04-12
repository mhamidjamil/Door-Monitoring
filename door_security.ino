/*************************************************************

  Rotate a servo using a slider and control an LED!

  App dashboard setup:
    Slider widget (0...180) on V3
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_TEMPELATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_AUTH_TOKEN"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

Servo servo;

int doorPin = 32; // Pin connected to the magnetic door switch

int sleep_led = 22;   // Pin connected to the LED
int warning_led = 21; // Pin connected to the LED
int monitoring_ = 33; // Pin connected to the LED

bool monitoring_status = false;
BLYNK_WRITE(V3) { servo.write(param.asInt()); }

BLYNK_WRITE(V7) {
  int monitor = param.asInt();
  if (monitor == 1) {
    monitoring_status = true;
    digitalWrite(monitoring_, HIGH);
  } else {
    monitoring_status = false;
    digitalWrite(monitoring_, LOW);
  }
}

void setup() {
  // Debug console
  Serial.begin(115200);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  servo.attach(4);

  pinMode(doorPin, INPUT_PULLUP);
  pinMode(sleep_led, OUTPUT);
  pinMode(warning_led, OUTPUT);
  pinMode(monitoring_, OUTPUT);
}

void loop() {
  Blynk.run();

  if (Blynk.connected()) {
    // Check monitoring mode status
    if (monitoring_status) {
      // Monitoring mode is active, check door state and update virtual pin 5
      // and 4
      if (digitalRead(doorPin) == LOW) {
        digitalWrite(sleep_led, LOW);
        digitalWrite(warning_led, LOW);
        Blynk.virtualWrite(V4, 1);
        Blynk.virtualWrite(V5, 1);
      } else {
        digitalWrite(warning_led, HIGH);
        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(V5, 0);
      }
    } else {
      // Monitoring mode is not active, check door state and update LED and
      // virtual pin 5
      digitalWrite(warning_led, LOW);
      if (digitalRead(doorPin) == LOW) {
        digitalWrite(sleep_led, LOW);
      } else {
        digitalWrite(sleep_led, HIGH);
      }
      Blynk.virtualWrite(V5, !digitalRead(doorPin));
    }
  }
}
