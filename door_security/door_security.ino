// Battery will charge when ac input is present
#include "config.h"

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
#include <WiFiClient.h>

#elif defined(ESP32)
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <WiFiClient.h>

#endif

Servo servo;

#define RED_LED 5
#define GREEN_LED 4
#define BLUE_LED 0
#define SERVO_PIN 2

#define DOOR_PIN 14
#define BUTTON_PIN 12

#define DOOR_CLOSE 180
#define DOOR_OPEN 0

bool door_status = false;

unsigned long savedTimes[3] = {0, 0, 0};

#define CHARGING_RELAY 9 // output
#define AC_INPUT 10      // input
unsigned int previous_time = 0;
unsigned long accumulatedTime = 0;
bool outputPinState = false;

int current_angle = 0;

bool monitoring_status = false;
BLYNK_WRITE(V3) {
  current_angle = param.asInt();
  servo.write(current_angle);
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
  pinMode(CHARGING_RELAY, OUTPUT);
  pinMode(AC_INPUT, INPUT);
}

void loop() {
  Blynk.run();
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    serialManager(command);
  }
  if (Blynk.connected()) {
    // Check monitoring mode status
    if (monitoring_status) {
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, HIGH);
      if (door_close()) {
        if (!door_status) {
          take_door_timeSnap();
          door_status = true;
        }
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
  battery_manager();
}
void DELAY(int delay_time) {
  for (int i = 0; i < delay_time; i += 10) {
    delay(10);
    button_event();
  }
}
void Blink(int led_number, int delay_time) {
  digitalWrite(led_number, HIGH);
  DELAY(delay_time / 2);
  digitalWrite(led_number, LOW);
  DELAY(delay_time / 2);
}
void Blink(int led_number, int delay_time, int blink_times) {
  for (int i = 0; i < blink_times; i++) {
    Blink(led_number, delay_time);
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
  // useless part => future work (use to generate trigger event)
  if (millis() - savedTimes[0] < 3000) {
    power_off();
    // close door after blinking blue led for 3 seconds
    Blink(BLUE_LED, 500, 5);
    if (door_close()) {
      servo.write(DOOR_CLOSE);
      current_angle = DOOR_CLOSE;
    } else {
      if (monitoring_status) {
        Blink(GREEN_LED, 500, 5);
      } else {
        Blink(RED_LED, 500, 5);
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
  DELAY(3000);
  if (door_close() && current_angle != DOOR_CLOSE) {
    power_off();
    Blink(GREEN_LED, 500, 5);
    servo.write(DOOR_CLOSE);
    current_angle = DOOR_CLOSE;
  }
}
void button_event() {
  if (digitalRead(BUTTON_PIN) == HIGH) {
    delay(1000);
    if (current_angle == DOOR_CLOSE) {
      servo.write(DOOR_OPEN);
      current_angle = DOOR_OPEN;
      Blink(GREEN_LED, 500, 2);
    } else {
      servo.write(DOOR_CLOSE);
      current_angle = DOOR_CLOSE;
      Blink(RED_LED, 500, 2);
    }
  }
}
void serialManager(String command) {
  if (command == "open") {
    servo.write(DOOR_OPEN);
    current_angle = DOOR_OPEN;
    Serial.println("Door is open");
  } else if (command == "close") {
    servo.write(DOOR_CLOSE);
    current_angle = DOOR_CLOSE;
    Serial.println("Door is close");
  } else if (command == "monitor") {
    monitoring_status = true;
    digitalWrite(BLUE_LED, HIGH);
    Serial.println("Monitoring mode is on");
  } else if (command == "stop") {
    monitoring_status = false;
    digitalWrite(BLUE_LED, LOW);
    Serial.println("Monitoring mode is off");
  } else if (command == "status") {
    Serial.println("Door status: " + String(door_close()));
    Serial.println("Monitoring status: " + String(monitoring_status));
  } else {
    Serial.println("Command not found");
  }
}

void battery_manager() {
  delay(1000);
  if (digitalRead(AC_INPUT) == LOW) {
    accumulatedTime_manager(1);
    if (outputPinState) {
      digitalWrite(CHARGING_RELAY, LOW);
      outputPinState = false;
    }
  } else if (digitalRead(AC_INPUT) == HIGH) {
    if (accumulatedTime > 0) {
      digitalWrite(CHARGING_RELAY, HIGH);
      outputPinState = true;
      accumulatedTime_manager(-1);
    } else {
      digitalWrite(CHARGING_RELAY, LOW);
      outputPinState = false;
      previous_time = (millis() / 1000);
    }
  }
}
void accumulatedTime_manager(int change) {
  if (previous_time != (millis() / 1000)) {
    accumulatedTime =
        accumulatedTime + change * ((millis() / 1000) - previous_time);
    previous_time = millis() / 1000;
  }
}