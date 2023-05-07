// #8 Relays are still struggling
// working on custom Relay board
#include <Servo.h>

Servo servo;

#define AC_INPUT 12          // input
#define RELAY_MANAGER_PIN 11 // transistor on this pin
#define SERVO_BACKUP_PIN 10  // output
#define RED_LED 9
#define GREEN_LED 8
#define BLUE_LED 7
#define SERVO_PIN 6

#define DOOR_PIN 3
#define BUTTON_PIN 2
#define CHARGING_RELAY 5 // output
#define SWITCHER 4       // output -> switch between battery and adapter

#define DOOR_CLOSE 180
#define DOOR_OPEN 0
bool door_status = false;
bool DEBUGGING = true;

int current_angle = 0;

bool monitoring_status = false;

// #-----------------
// Battery function

unsigned int previous_time = 0;
unsigned long accumulatedTime = 0;
bool outputPinState = true;
// #-----------------

String command = "";
// BLYNK_WRITE(V3) {
//   current_angle = param.asInt();
//   servo.write(current_angle);
// }

// BLYNK_WRITE(V7) {
//   int monitor = param.asInt();
//   if (monitor == 1) {
//     monitoring_status = true;
//     digitalWrite(BLUE_LED, HIGH);
//   } else {
//     monitoring_status = false;
//     digitalWrite(BLUE_LED, LOW);
//   }
// }
void DELAY(int delay_time);
void setup() {
  // Debug console
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  // Serial.println("void setup enter point 1");
  // delay(500);
  // Blynk.begin(auth, ssid, pass);
  // Serial.println("void setup After blynk");

  servo.attach(SERVO_PIN);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(CHARGING_RELAY, OUTPUT);
  pinMode(AC_INPUT, INPUT);
  pinMode(RELAY_MANAGER_PIN, OUTPUT);
  pinMode(SERVO_BACKUP_PIN, OUTPUT);
  println("leaving void setup");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // Blynk.run();
  println("I'm alive :) " + String(millis() / 1000));
  DELAY(1000);
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    serialManager(command);
  }
  // if (Blynk.connected() || true) {
  if (true) {
    // Check monitoring mode status
    if (monitoring_status) {
      digitalWrite(BLUE_LED, HIGH);
      if (door_close()) {
        if (!door_status) {
          door_status = true;
        }
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, LOW);
        // Blynk.virtualWrite(V4, 1);
        // Blynk.virtualWrite(V5, 1);
      } else {
        if (door_status) {
          door_status = false;
        }
        digitalWrite(RED_LED, HIGH);
        // Blynk.virtualWrite(V4, 0);
        // Blynk.virtualWrite(V5, 0);
      }
    } else {
      digitalWrite(RED_LED, LOW);
      if (door_close()) {
        if (!door_status) {
          door_status = true;
        }
        digitalWrite(GREEN_LED, LOW);
      } else {
        if (door_status) {
          door_status = false;
        }
        digitalWrite(GREEN_LED, HIGH);
      }
      // Blynk.virtualWrite(V5, !digitalRead(DOOR_PIN));
    }
  }
  print("After true,\t");
  battery_manager();
  println("After battery manager");
  force_door_off();
  print("After force off\t");
}
void DELAY(int delay_time) {
  for (int i = 0; i < delay_time; i += 10) {
    delay(10);
    button_event();
  }
}
void Blink(int led_number, int delay_time) {
  digitalWrite(led_number, HIGH);
  delay(delay_time / 2);
  digitalWrite(led_number, LOW);
  delay(delay_time / 2);
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
void power_off() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}
void force_door_off() {
  if (door_close() && current_angle != DOOR_CLOSE) {
    Serial.println("Closing door...>");
    delay(3000);
    power_off();
    Blink(GREEN_LED, 500, 5);
    servo.write(DOOR_CLOSE);
    current_angle = DOOR_CLOSE;
  }
}
void button_event() {
  // Blink(GREEN_LED, 500, 5);
  if (digitalRead(BUTTON_PIN) == HIGH) {
    println("---------------------------------------------------");
    println("Button pressed");
    println("---------------------------------------------------");
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
  if (command.indexOf("#open") != -1) {
    servo.write(DOOR_OPEN);
    current_angle = DOOR_OPEN;
    Serial.println("Door is open");
  } else if (command.indexOf("#close") != -1) {
    servo.write(DOOR_CLOSE);
    current_angle = DOOR_CLOSE;
    Serial.println("Door is close");
  } else if (command.indexOf("#monitoring") != -1) {
    monitoring_status = true;
    digitalWrite(BLUE_LED, HIGH);
    Serial.println("Monitoring mode is on");
  } else if (command.indexOf("#stop") != -1) {
    monitoring_status = false;
    digitalWrite(BLUE_LED, LOW);
    Serial.println("Monitoring mode is off");
  } else if (command.indexOf("#status") != -1) {
    Serial.println("Door status: " + String(door_close()));
    Serial.println("Monitoring status: " + String(monitoring_status));
  } else if (command.indexOf("#battery") != -1) {
    println(F("*************************************"));
    println("String recived : " + command);

    int dotIndex = command.indexOf('.');
    int exclamationIndex = command.indexOf('!');
    if (dotIndex != -1 && exclamationIndex != -1 &&
        exclamationIndex > dotIndex + 1) {
      String batteryLevelString =
          command.substring(dotIndex + 1, exclamationIndex);
      accumulatedTime += batteryLevelString.toInt();
      println("Required time : " + String(accumulatedTime) + " Seconds");
    } else {
      println("Invalid input string!");
    }

    print(F("\n*************************************"));
  } else {
    Serial.println("Command not found");
  }
}
void battery_manager() {
  Serial.println("AC_INPUT: " + String(digitalRead(AC_INPUT)) +
                 " accumulatedTime: " + String(accumulatedTime) +
                 +" previous_time: " + String(previous_time));
  // delay(1000);
  if (digitalRead(AC_INPUT) == LOW) {
    accumulatedTime_manager(1);
    if (outputPinState) {
      digitalWrite(SERVO_BACKUP_PIN, HIGH);
      delay(2000);

      digitalWrite(RELAY_MANAGER_PIN, HIGH);
      delay(2000);
      digitalWrite(CHARGING_RELAY, HIGH);
      delay(3000);

      digitalWrite(RELAY_MANAGER_PIN, LOW);
      digitalWrite(CHARGING_RELAY, LOW);

      outputPinState = false;
    }
  } else if (digitalRead(AC_INPUT) == HIGH) {
    digitalWrite(SERVO_BACKUP_PIN, LOW);
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
  int temp_accumulatedTime =
      accumulatedTime + change * ((millis() / 1000) - previous_time);
  previous_time = millis() / 1000;
  if (temp_accumulatedTime < 0) {
    accumulatedTime = 0;
  } else {
    accumulatedTime = temp_accumulatedTime;
  }
}
void println(String input) {
  if (DEBUGGING)
    print(input + "\n");
}
void print(String input) {
  if (DEBUGGING)
    Serial.print(input);
}