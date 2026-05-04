void setup() {
  // Start serial communication at 9600 bits per second
  Serial.begin(9600); 
}

void loop() {
  // Print "Hello World!" followed by a new line
  Serial.println("Hello World!");
  // Wait for 1 second (1000 milliseconds)
  delay(1000); 
}