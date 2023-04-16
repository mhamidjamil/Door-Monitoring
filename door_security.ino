
/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL6JjS0X4Ey"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "V0GUIMD1pXGciOfg6LUB7Xd6rhS4iRPp"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#if defined(ESP8266)
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#include <Servo.h>
#include <WiFiClient.h>

#elif defined(ESP32)
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>

#endif

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "eduroam";
char pass[] = "Room@102+$";

Servo servo;

#define BUTTON_PIN 12
#define GREEN_LED 4
#define RED_LED 5
#define BLUE_LED 0
#define SERVO_PIN 2

#define DOOR_PIN 14
#define DOOR_CLOSE 180
#define DOOR_OPEN 0
bool door_status = false;
unsigned long savedTimes[3] = {0, 0, 0};

int current_angle = 0;

bool monitoring_status = false;
BLYNK_WRITE(V3) {
  current_angle = param.asInt();
  servo.write(current_angle);
  // Serial.print("Current angle: " + String(current_angle));
}

BLYNK_WRITE(V7) {
  int monitor = param.asInt();
  if (monitor == 1) {
    monitoring_status = true;
    digitalWrite(BLUE_LED, HIGH);
  } else {
    monitoring_status = false;
    digitalWrite(BLUE_LED, LOW);
  }
}
void DELAY(int delay_time);
void setup() {
  // Debug console
  Serial.begin(9600);
  // You can also specify server:
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  // Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

#if defined(ESP32)
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
#elif defined(ESP8266)
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    DELAY(500);
    Serial.print(".");
  }
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);

#endif

  servo.attach(SERVO_PIN);

  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  Blynk.run();

  if (Blynk.connected()) {
    // Check monitoring mode status
    if (monitoring_status) {
      digitalWrite(BLUE_LED, HIGH);
      if (door_close()) {
        if (!door_status) {
          take_door_timeSnap();
          door_status = true;
        }
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, LOW);
        Blynk.virtualWrite(V4, 1);
        Blynk.virtualWrite(V5, 1);
      } else {
        if (door_status) {
          take_door_timeSnap();
          door_status = false;
        }
        digitalWrite(RED_LED, HIGH);
        Blynk.virtualWrite(V4, 0);
        Blynk.virtualWrite(V5, 0);
      }
    } else {
      digitalWrite(RED_LED, LOW);
      if (door_close()) {
        if (!door_status) {
          take_door_timeSnap();
          door_status = true;
        }
        digitalWrite(GREEN_LED, LOW);
      } else {
        if (door_status) {
          take_door_timeSnap();
          door_status = false;
        }
        digitalWrite(GREEN_LED, HIGH);
      }
      Blynk.virtualWrite(V5, !digitalRead(DOOR_PIN));
    }
  }
  force_door_off();
}
void DELAY(int delay_time) {
  for (int i = 0; i < delay_time; i += 10) {
    delay(10);
    if (digitalRead(BUTTON_PIN) == HIGH) {
      if (current_angle == 0) {
        current_angle = 180;
      } else {
        current_angle = 0;
      }
      servo.write(current_angle);
    }
  }
}
void blink(int led_number, int delay_time) {
  digitalWrite(led_number, HIGH);
  DELAY(delay_time / 2);
  digitalWrite(led_number, LOW);
  DELAY(delay_time / 2);
}
void blink(int led_number, int delay_time, int blink_times) {
  for (int i = 0; i < blink_times; i++) {
    blink(led_number, delay_time);
  }
}
bool door_close() {
  if (digitalRead(DOOR_PIN) == LOW) {
    return true;
  } else {
    return false;
  }
}
void take_door_timeSnap() {
  if (millis() - savedTimes[0] < 3000) {
    power_off();
    // close door after blinking blue led for 3 seconds
    blink(BLUE_LED, 500, 5);
    if (door_close()) {
      servo.write(DOOR_CLOSE);
      current_angle = DOOR_CLOSE;
    } else {
      if (monitoring_status) {
        blink(GREEN_LED, 500, 5);
      } else {
        blink(RED_LED, 500, 5);
      }
    }
  } else {
    savedTimes[0] = savedTimes[1];
    savedTimes[1] = savedTimes[2];
    savedTimes[2] = millis();
  }
}
void power_off() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}
void force_door_off() {
  if (door_close() && current_angle != DOOR_CLOSE) {
    power_off();
    blink(GREEN_LED, 500, 5);
    servo.write(DOOR_CLOSE);
    current_angle = DOOR_CLOSE;
  }
}