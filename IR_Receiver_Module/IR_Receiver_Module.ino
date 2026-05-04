#include <IRremote.hpp>

const int IR_RECEIVE_PIN = 9;

void setup() {
  Serial.begin(9600);
  
  // The modern v4.x initialization
  // ENABLE_LED_FEEDBACK will actually blink the built-in Arduino LED (Pin 13) when it sees a signal
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  
  Serial.println("IR Receiver V4.x Diagnostics Online.");
  Serial.println("Press a button on the remote...");
}

void loop() {
  // The modern v4.x decode function
  if (IrReceiver.decode()) {
    
    // Ignore repeat codes for cleaner logging
    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
      
      Serial.print("Protocol: ");
      Serial.print(IrReceiver.decodedIRData.protocol);
      Serial.print(" | Command: 0x");
      Serial.println(IrReceiver.decodedIRData.command, HEX);
      
    }
    
    // Prime the receiver for the next signal
    IrReceiver.resume(); 
  }
}