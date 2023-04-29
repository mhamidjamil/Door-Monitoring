#define CHARGING_RELAY 9 // output
#define AC_INPUT 10      // input
unsigned int previous_time = 0;
unsigned long accumulatedTime = 0;
bool outputPinState = false;

void setup() {
  Serial.begin(9600);
  pinMode(CHARGING_RELAY, OUTPUT);
  pinMode(AC_INPUT, INPUT);
}
void loop() { battery_manager(); }

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
// void checkInputPin() {
//   unsigned long currentTime = millis();
//   bool inputPinState = digitalRead(INPUT_PIN);

//   if (inputPinState == LOW) {
//     startTime = currentTime;
//   } else if (inputPinState == HIGH && startTime != 0) {
//     accumulatedTime += (currentTime - startTime) / 1000;
//     startTime = 0;
//     if (outputPinState) {
//       digitalWrite(OUTPUT_PIN, LOW);
//       outputPinState = false;
//     }
//   }

//   if (accumulatedTime > 0 && !outputPinState) {
//     digitalWrite(OUTPUT_PIN, HIGH);
//     outputPinState = true;
//     startTime = currentTime;
//   }

//   if (outputPinState && (currentTime - startTime) / 1000 >= accumulatedTime)
//   {
//     digitalWrite(OUTPUT_PIN, LOW);
//     outputPinState = false;
//     accumulatedTime = 0;
//     startTime = 0;
//   }
// }