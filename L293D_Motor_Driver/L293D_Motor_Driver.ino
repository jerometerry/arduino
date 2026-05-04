void setup() {
  // Set the logic pin as an output
  pinMode(10, OUTPUT);
}

void loop() {
  // Drive the motor forward
  digitalWrite(10, HIGH);
  delay(2000);
  
  // Stop the motor
  digitalWrite(10, LOW);
  delay(2000);
}