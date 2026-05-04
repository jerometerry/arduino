// --- Digital Input Diagnostic Tool ---

const int TEST_PIN = 9;
int lastButtonState = HIGH; // Default state because of Pull-Up

void setup() {
  Serial.begin(9600);
  
  // Activate the internal pull-up resistor
  pinMode(TEST_PIN, INPUT_PULLUP);
  
  Serial.println("Digital Input Diagnostic Online.");
  Serial.println("Listening on Pin 9...");
}

void loop() {
  // Read the current state of the pin
  int buttonState = digitalRead(TEST_PIN);

  // Check if the state has changed from HIGH to LOW (Button Pressed)
  if (buttonState == LOW && lastButtonState == HIGH) {
    Serial.println("SUCCESS: Hardware Input Detected on Pin 9!");
    
    // A tiny delay to "debounce" the physical spring inside the button
    delay(50); 
  }

  // Save the state for the next loop
  lastButtonState = buttonState;
}