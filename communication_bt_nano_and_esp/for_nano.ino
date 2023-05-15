void setup() {
  // Initialize serial communication
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('!');
    
    if (command.startsWith("#")) {
      if (command.equals("#open")) {
        // Perform open command action
        Serial.println("Command received: Open");
      } else if (command.equals("#close")) {
        // Perform close command action
        Serial.println("Command received: Close");
      } else if (command.equals("#status")) {
        // Perform status command action
        Serial.println("Command received: Status");
      } else {
        // Unknown command
        Serial.println("Unknown command");
      }
    }
  }
  
  // Your other code logic goes here
}
