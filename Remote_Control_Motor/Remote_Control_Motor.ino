#define DECODE_NEC 1
#define EXCLUDE_EXOTIC_PROTOCOLS 1
#define NO_LED_SEND_FEEDBACK_CODE 1

#include <IRremote.hpp>

// --- Elegoo Super Starter Kit Button Mappings
const uint16_t CMD_UP    = 0x9;
const uint16_t CMD_DOWN  = 0x7;
const uint16_t CMD_PAUSE = 0x40;
const uint16_t CMD_POWER = 0x45;
const uint16_t CMD_STOP  = 0x47;

// --- Hardware Pin Definitions ---
const int IR_RECEIVER_PIN = 9;  
const int MOTOR_IN1_PIN = 11;   
const int MOTOR_IN2_PIN = 10;

const unsigned long baudRate = 9600;

// --- Motor Logical State ---
bool isRunning = false;
bool isForward = true;

void setup() {
  enableSerialPort();  
  initializeMotorInterface();
  enableIrReceiver();
  printWaitingForInputMessage();
}

void enableSerialPort() {
  Serial.begin(baudRate);
}

void initializeMotorInterface() {
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  stopMotor();
}

void enableIrReceiver() {
  IrReceiver.begin(IR_RECEIVER_PIN, ENABLE_LED_FEEDBACK);
}

void printWaitingForInputMessage() {
  Serial.println("L293D V4.x Controller Ready");
  Serial.println("IR Remote Usage:");
  Serial.println("Up button: spin engine forward");
  Serial.println("Down button: spin engine in reverse");
  Serial.println("Power button: start/resume/stop motor");
  Serial.println("Pause button: start/resume/stop motor");
  Serial.println("Func/Stop button: stop motor");
}

void loop() {
  if (IrReceiver.decode()) {
    if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
      uint16_t command = IrReceiver.decodedIRData.command;
      executeCommand(command);
    }
    IrReceiver.resume(); 
  }
}

void executeCommand(uint16_t cmd) {
  if (!isValidCommand(cmd)) {
    return;
  }

  Serial.print("Received Command: 0x");
  Serial.print(cmd, HEX);  

  switch (cmd) {
    case CMD_UP: 
      Serial.println(" -> Command: UP. Direction set FORWARD.");
      spinForward();
      break;
    case CMD_DOWN: 
      Serial.println(" -> Command: DOWN. Direction set REVERSE.");
      spinReverse();
      break;
    case CMD_STOP: 
      Serial.println(" -> Command: STOP. Applying BRAKE.");
      stopMotor();
      break;
    case CMD_POWER:
    case CMD_PAUSE:
      togglePower();
      break;
    default:
      Serial.println(" -> Command unmapped. Ignoring.");
      break;
  }
}

bool isValidCommand(uint16_t cmd) {
  return cmd != 0x0;
}

void stopMotor() {
  isRunning = false;
  digitalWrite(MOTOR_IN1_PIN, LOW);
  digitalWrite(MOTOR_IN2_PIN, LOW);
}

void spinForward() {
  isForward = true;
  isRunning = true;
  digitalWrite(MOTOR_IN1_PIN, HIGH);
  digitalWrite(MOTOR_IN2_PIN, LOW);
}

void spinReverse() {
  isForward = false;
  isRunning = true;
  digitalWrite(MOTOR_IN1_PIN, LOW);
  digitalWrite(MOTOR_IN2_PIN, HIGH);
}

void togglePower() {
  isRunning = !isRunning;

  Serial.print(" -> Command: TOGGLE. ");
  if (isRunning) {
    Serial.println(isForward ? "Resuming FORWARD." : "Resuming REVERSE.");
    if (isForward) {
      spinForward();
    } else {
      spinReverse();
    }
  } else {
    Serial.println("Stopping");
    stopMotor();
  }
}