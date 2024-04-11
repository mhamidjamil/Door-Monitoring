#define RF_PIN1 36
#define RF_PIN2 39
#define RF_PIN3 34
#define RF_PIN4 35
int globPin1Value = 0;
int globPin2Value = 0;
int globPin3Value = 0;
int globPin4Value = 0;

String targetStr = "0111";

void setup() {
  Serial.begin(115200);
  pinMode(RF_PIN1, INPUT);
  pinMode(RF_PIN2, INPUT);
  pinMode(RF_PIN3, INPUT);
  pinMode(RF_PIN4, INPUT);
}

void loop() {
  int Pin1ValueTemp = digitalRead(RF_PIN1);
  int Pin2ValueTemp = digitalRead(RF_PIN2);
  int Pin3ValueTemp = digitalRead(RF_PIN3);
  int Pin4ValueTemp = digitalRead(RF_PIN4);
  if (Pin1ValueTemp != globPin1Value || Pin2ValueTemp != globPin2Value ||
      Pin3ValueTemp != globPin3Value || Pin4ValueTemp != globPin4Value) {
    globPin1Value = Pin1ValueTemp;
    globPin2Value = Pin2ValueTemp;
    globPin3Value = Pin3ValueTemp;
    globPin4Value = Pin4ValueTemp;
    Serial.println("->" + String(globPin1Value) + "," + String(globPin2Value) +
                   "," + String(globPin3Value) + "," + String(globPin4Value) +
                   ".");
  }
  if (String(globPin1Value) + String(globPin2Value) + String(globPin3Value) +
          String(globPin4Value) ==
      targetStr) {
    Serial.println("\n\t\t ! => Opening Gate");
  }
  delay(500);
}