#define AC_INPUT 10       // input
#define CHARGING_RELAY 13 // output
#define SWITCHER 15       // output -> switch between battery and adapter

unsigned int previous_time = 0;
unsigned long accumulatedTime = 0;
bool outputPinState = false;

void setup() {
  Serial.begin(9600);
  pinMode(CHARGING_RELAY, OUTPUT);
  pinMode(AC_INPUT, INPUT);
  pinMode(SWITCHER, OUTPUT);
}
void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    serialManager(command);
  }
  battery_manager();
}

void battery_manager() {
  Serial.println("AC_INPUT: " + String(digitalRead(AC_INPUT)) +
                 " accumulatedTime: " + String(accumulatedTime) +
                 +" previous_time: " + String(previous_time));
  delay(1000);
  if (digitalRead(AC_INPUT) == LOW) {
    digitalWrite(SWITCHER, HIGH);
    accumulatedTime_manager(1);
    if (outputPinState) {
      digitalWrite(CHARGING_RELAY, HIGH);
      outputPinState = false;
    }
  } else if (digitalRead(AC_INPUT) == HIGH) {
    digitalWrite(SWITCHER, LOW);
    if (accumulatedTime > 0) {
      digitalWrite(CHARGING_RELAY, LOW);
      outputPinState = true;
      accumulatedTime_manager(-1);
    } else {
      digitalWrite(CHARGING_RELAY, HIGH);
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
//.........................
void serialManager(String command) {
  if (command.indexOf("#open") != -1) {
    // servo.write(DOOR_OPEN);
    // current_angle = DOOR_OPEN;
    Serial.println("Door is open");
  } else if (command.indexOf("#close") != -1) {
    // servo.write(DOOR_CLOSE);
    // current_angle = DOOR_CLOSE;
    Serial.println("Door is close");
  } else if (command.indexOf("#monitor") != -1) {
    // monitoring_status = true;
    // digitalWrite(BLUE_LED, HIGH);
    Serial.println("Monitoring mode is on");
  } else if (command.indexOf("#stop") != -1) {
    // monitoring_status = false;
    // digitalWrite(BLUE_LED, LOW);
    Serial.println("Monitoring mode is off");
  } else if (command.indexOf("#status") != -1) {
    // Serial.println("Door status: " + String(door_close()));
    // Serial.println("Monitoring status: " + String(monitoring_status));
  } else {
    Serial.println("Command not found");
    Serial.println("-> " + command + " <-");
  }
}