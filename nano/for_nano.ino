//$ build 2.0.2
//* last work: 2023-may-20 12:40PM
// # nano have to link with esp8266 for wifi usage
#include <Servo.h>

Servo servo;

#define AC_INPUT 12 // check if the adapter is connected and power's on
#define LED_ 11     // force power bank to power on
#define BATTERY_TO_SERVO_PIN 10
// Servo power up with battery when adapter isn't connected
#define RED_LED 9
#define GREEN_LED 8
#define BLUE_LED 7
#define SERVO_PIN 6
#define CHARGING_RELAY 5 // responsible for charging the battery
#define SWITCHER 4       // output -> switch between battery and adapter
#define DOOR_PIN 3       //  pin will check either door is open or close
#define BUTTON_PIN 2     // Button to alter servo angel

#define DOOR_CLOSE 180
#define DOOR_OPEN 60 // 60 angle in enough for my door lock
bool door_status = false;
bool DEBUGGING = false; // additional outputs will displayed when true

int current_angle = 0;

bool monitoring_status = false;

#define LIMIT 99
#define DOOR_DELAY 3
byte pin_second = 0;

// #-----------------
// Battery function

unsigned int previous_time = 0;
unsigned long accumulatedTime = 0;
bool outputPinState = true;
// #-----------------

String command = "";

void DELAY(int delay_time);
void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);

  servo.attach(SERVO_PIN);
  DOOR(false);
  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(CHARGING_RELAY, OUTPUT);
  pinMode(AC_INPUT, INPUT);
  pinMode(LED_, OUTPUT);
  pinMode(BATTERY_TO_SERVO_PIN, OUTPUT);
  println("leaving void setup");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  // println("I'm alive :) " + String(millis() / 1000));
  Delay(1000);
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    serialManager(command);
  }
  if (monitoring_status) {
    digitalWrite(BLUE_LED, HIGH);
    if (door_close()) {
      if (!door_status) {
        door_status = true;
      }
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(RED_LED, LOW);
    } else {
      if (door_status) {
        door_status = false;
      }
      digitalWrite(RED_LED, HIGH);
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
  }
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
    return true; // door is closed
  } else {
    return false; // door is open
  }
}
void power_off() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}
void force_door_off() {
  println("before first condition");
  if (((millis() / 1000) >= pin_second) &&
          ((((millis() / 1000) % 100) + (DOOR_DELAY + 2)) < LIMIT) ||
      pin_second == 0) {
    pin_second = 0;
    println("Entered in second condition");
    if (door_close() && current_angle != DOOR_CLOSE) {
      Serial.println("Closing door...>");
      power_off();
      Blink(GREEN_LED, 150, 4);
      DOOR(false);
    } else {
      println("\n$$$$$$$$\nDoor close failed; logs \n : c_angle: " +
              String(current_angle) + ", d_close() : " + String(door_close()) +
              "\n$$$$$$$$\n");
    }
  } else {
    if (current_angle == DOOR_CLOSE) {
      println("Door is already closed");
      pin_second = 0;
    }
    println("*********************");
    println("1st condition fail logs: ");
    println("millis : " + String(millis() / 1000) + "\t" +
            "pin_second : " + pin_second);
    println("*********************");
  }
}
void button_event() {
  // Blink(GREEN_LED, 500, 5);
  if (digitalRead(BUTTON_PIN) == HIGH) {
    println("$-----------------------$");
    println("     Button pressed");
    println("$-----------------------$");
    delay(200);
    if (current_angle == DOOR_CLOSE) {
      DOOR(true, true);
      Blink(GREEN_LED, 500, 2);
    } else {
      DOOR(false);
      Blink(RED_LED, 500, 2);
    }
  }
}
void serialManager(String command) {
  if (command.indexOf("#open") != -1) {
    DOOR(true, true);
    Serial.println("Door is open");
  } else if (command.indexOf("#close") != -1) {
    DOOR(false);
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
  } else if (command.indexOf("#debug") != -1) {
    DEBUGGING = !DEBUGGING;
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

    println(F("*************************************"));
  } else if (command.indexOf("#servo") != -1) {
    int dotIndex = command.indexOf('.');
    int exclamationIndex = command.indexOf('!');
    if (dotIndex != -1 && exclamationIndex != -1 &&
        exclamationIndex > dotIndex + 1) {
      String servoAngleString =
          command.substring(dotIndex + 1, exclamationIndex);
      servo.write(servoAngleString.toInt());
      println("Servo angle : " + String(servoAngleString.toInt()));
    } else {
      println("Invalid input string!");
    }
  } else {
    Serial.println("Command not found");
  }
}
void battery_manager() {
  println(("_" + String(digitalRead(AC_INPUT)) + "_ AT: " +
           String(accumulatedTime) + +" PT: " + String(previous_time)),
          1000); // AT = Accumulated Time, PT = Previous Time

  // delay(1000);
  if (digitalRead(AC_INPUT) == LOW) {
    accumulatedTime_manager(1);
    if (outputPinState) {
      digitalWrite(LED_, HIGH);
      delay(2000);
      digitalWrite(CHARGING_RELAY, HIGH);
      delay(300);

      digitalWrite(LED_, LOW);
      digitalWrite(CHARGING_RELAY, LOW);
      digitalWrite(BATTERY_TO_SERVO_PIN, HIGH);

      outputPinState = false;
    }
  } else if (digitalRead(AC_INPUT) == HIGH) {
    if (millis() / 1000 < 10) {
      accumulatedTime = 2000; // charge battery for half hour.
    }
    digitalWrite(BATTERY_TO_SERVO_PIN, LOW);
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
void force_door_off() {
  println("before first condition");
  if (((millis() / 1000) >= pin_second) &&
          (!(((millis() / 1000) % 100) > (LIMIT - (DOOR_DELAY + 2)))) ||
      pin_second == 0) { //` second part of condition is not fine
    pin_second = 0;
    println("Entered in second condition");
    if (door_close() && current_angle != DOOR_CLOSE) {
      Serial.println("Closing door...>");
      power_off();
      Blink(GREEN_LED, 150, 4);
      DOOR(false);
    } else {
      println("\n$$$$$$$$\nDoor close failed; logs \n : c_angle: " +
              String(current_angle) + ", d_close() : " + String(door_close()) +
              "\n$$$$$$$$\n");
    }
  } else {
    if (current_angle == DOOR_CLOSE) {
      println("Door is already closed");
      pin_second = 0;
    }
    println("*********************");
    println("1st condition fail logs: ");
    println("millis : " + String(millis() / 1000) + "\t" +
            "pin_second : " + pin_second);
    println("*********************");
  }
}
void button_event() {
  // Blink(GREEN_LED, 500, 5);
  if (digitalRead(BUTTON_PIN) == HIGH) {
    println("$-----------------------$");
    println("     Button pressed");
    println("$-----------------------$");
    delay(200);
    if (current_angle == DOOR_CLOSE) {
      DOOR(true, true);
      Blink(GREEN_LED, 500, 2);
    } else {
      DOOR(false);
      Blink(RED_LED, 500, 2);
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
void DOOR(bool status) { // true = open, false = close
  if (status) {
    servo.write(DOOR_OPEN);
    current_angle = DOOR_OPEN;
  } else {
    servo.write(DOOR_CLOSE);
    current_angle = DOOR_CLOSE;
  }
}
void DOOR(bool status, bool time_allotment) {
  //` 1: (true = open, false = close),
  // # 2: time_stamp if true it will allocated time stamp to `pin_second`
  DOOR(status);
  delay(700);
  do {
    if ((((millis() / 1000) % 100) + DOOR_DELAY) > LIMIT) {
      pin_second = (DOOR_DELAY - 1) - (LIMIT - (millis() % 100));
    } else {
      pin_second = ((millis() / 1000) % 100) + DOOR_DELAY;
    }
    println("Initializing pin seconds to -> " + String(pin_second) +
            "from millis : " + String((millis() / 1000) % 100));
  } while (time_allotment && door_close());
  println("leaving DOOR function with pin_second value :  " +
          String(pin_second));
}
void Delay(int delay_time) {
  if (DEBUGGING) {
    DELAY(delay_time);
  } else {
    DELAY(50);
  }
}
void println(String str, int delay_time) {
  println(str);
  DELAY(delay_time);
}
void println(String input) {
  if (DEBUGGING)
    print(input + "\n");
}
void print(String input) {
  if (DEBUGGING)
    Serial.print(input);
}
void DELAY(int delay_time) {
  for (int i = 0; i < delay_time; i += 10) {
    delay(10);
    button_event();
  }
}
bool door_close() {
  if (digitalRead(DOOR_PIN) == LOW) {
    return true; // door is closed
  } else {
    return false; // door is open
  }
}
void Blink(int led_number, int delay_time, int blink_times) {
  for (int i = 0; i < blink_times; i++) {
    Blink(led_number, delay_time);
  }
}
void Blink(int led_number, int delay_time) {
  digitalWrite(led_number, HIGH);
  delay(delay_time / 2);
  digitalWrite(led_number, LOW);
  delay(delay_time / 2);
}
void power_off() {
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
}
