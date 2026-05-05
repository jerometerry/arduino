#include <Arduino.h>

#include "PinDefinitionsAndMore.h"

#if FLASHEND >= 0x7FFF // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
// Do not enable any protocol explicitly => all protocols are enabled automatically.
// !!! Enabling B&O disables detection of Sony, because the repeat gap for SONY is smaller than the B&O frame gap :-( !!!
// #define DECODE_BEO // Bang & Olufsen protocol always must be enabled explicitly. It has an IR transmit frequency of 455 kHz! It prevents decoding of SONY! ~ 430 bytes
#else
// for 8k flash
// #define DECODE_DENON        // Includes Sharp - requires around 250 bytes of program memory on ATmega328
// #define DECODE_JVC          // ~ 200 bytes
#define DECODE_KASEIKYO       // Includes Panasonic ~ 300 bytes
#define DECODE_LG             // ~ 400 bytes
#define DECODE_NEC            // Includes Apple and Onkyo ~ 250 bytes
#define DECODE_SAMSUNG        // ~ 300 bytes
// #define DECODE_SONY         // ~ 175 bytes
// #define DECODE_RC5          // RC5 + MARANTZ: ~ 425 bytes
// #define DECODE_RC6          // ~ 375 bytes
#define DECODE_DISTANCE_WIDTH // Universal decoder for pulse distance width protocols ~ 2275 bytes
#define DECODE_HASH           // special decoder for all protocols ~ 250 bytes

#define EXCLUDE_EXOTIC_PROTOCOLS // Saves around 90 bytes of program memory, if activated
#endif

#if !defined(RAW_BUFFER_LENGTH)
// Use more than the default values of 100 for 512 bytes RAM, 200 for 2k RAM and 750 for more than 2k RAM
#if RAMSIZE <= 0x400
// Here we have 1 k RAM or less
#define RAW_BUFFER_LENGTH 360
#elif RAMSIZE <= 0x800
// Here we have 2 k RAM or less, otherwise use default of 750
#define RAW_BUFFER_LENGTH 600 // 400 is OK with Pronto and 1000 is OK without Pronto. 1200 is too much here, because then variables are overwritten.
#endif
#endif

// #define NO_LED_FEEDBACK_CODE        // saves 92 bytes program memory
// #define EXCLUDE_UNIVERSAL_PROTOCOLS // Saves up to 1000 bytes program memory.
// #define EXCLUDE_EXOTIC_PROTOCOLS    // saves around 650 bytes program memory if all other protocols are active
// #define USE_THRESHOLD_DECODER       // May give slightly better results especially for jittering signals and protocols with short 1 pulses / pauses. Requires additional 1 bytes program memory.
// #define IR_REMOTE_DISABLE_RECEIVE_COMPLETE_CALLBACK // saves 32 bytes program memory
// #define USE_16_BIT_TIMING_BUFFER    // Use a 16-bit buffer to preserve values above 12750 us
#define SHOW_DISTANCE_WIDTH_DECODER_ERRORS // Prints the reason which prevents data to be decoded as distance width data

// MARK_EXCESS_MICROS is subtracted from all marks and added to all spaces before decoding,
// to compensate for the signal forming of different IR receiver modules. See also IRremote.hpp line 135.
// 20 is taken as default if not otherwise specified / defined.
// #define MARK_EXCESS_MICROS    40    // Adapt it to your IR receiver module. 40 is recommended for the cheap VS1838 modules at high intensity.

#if defined(DECODE_BEO)
#define RECORD_GAP_MICROS 16000 // always get the complete frame in the receive buffer, but this prevents decoding of SONY!
#endif
// #define RECORD_GAP_MICROS 12000 // Default is 8000. Activate it for some LG air conditioner protocols

// #define DEBUG // Activate this for lots of lovely debug output from the decoders.

#include <IRremote.hpp>

#if defined(APPLICATION_PIN) && !defined(DEBUG_BUTTON_PIN)
#define DEBUG_BUTTON_PIN APPLICATION_PIN // if held low, print timing for each received data
#else
#define DEBUG_BUTTON_PIN 6
#endif
#if defined(ESP32) && defined(DEBUG_BUTTON_PIN)
#if !digitalPinIsValid(DEBUG_BUTTON_PIN)
#undef DEBUG_BUTTON_PIN // DEBUG_BUTTON_PIN number is not valid, so delete definition to disable further usage
#endif
#endif



/*
 * Using the function printActiveIRProtocols() requires additional 314 bytes program memory
 * Using the function printIRResultShort() requires additional 1450 bytes program memory
 * Using the function printIRSendUsage() requires additional 2736 bytes program memory
 * Because these 3 functions all share common code, using all 3 functions requires only additional 3016 bytes program memory
 */

void generateTone();
void handleOverflow();
bool detectLongPress(uint16_t aLongPressDurationMillis);

// --- IR Telemetry Mappings ---
const uint16_t CMD_UP = 0x9;
const uint16_t CMD_DOWN = 0x7;
const uint16_t CMD_PAUSE = 0x40;
const uint16_t CMD_POWER = 0x45;
const uint16_t CMD_STOP = 0x47;

// --- Hardware Pin Definitions ---
#define IR_RECEIVE_PIN 11
const int MOTOR_IN1_PIN = 9;
const int MOTOR_IN2_PIN = 10;

// --- Motor Logical State ---
bool isRunning = false;
bool isForward = true; // Defaults to forward on boot

void setup()
{
#if FLASHEND >= 0x3FFF // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
#if defined(DEBUG_BUTTON_PIN)
  pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
#endif
#endif

  Serial.begin(115200);

#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/ || defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID) || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217) || (defined(ESP32) && defined(ARDUINO_USB_MODE))
  delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif

  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  // In case the interrupt driver crashes on setup, give a clue
  // to the user what's going on.
  Serial.println(F("Enabling IRin..."));

  // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  Serial.print(F("Ready to receive IR signals of protocols: "));
  printActiveIRProtocols(&Serial);
#if defined(IR_RECEIVE_PIN_STRING)
  Serial.println(F("at pin " IR_RECEIVE_PIN_STRING));
#else
  Serial.println(F("at pin " STR(IR_RECEIVE_PIN)));
#endif

#if defined(LED_BUILTIN) && !defined(NO_LED_FEEDBACK_CODE)
#if defined(FEEDBACK_LED_IS_ACTIVE_LOW)
  Serial.print(F("Active low "));
#endif
  Serial.print(F("FeedbackLED at pin "));
  Serial.println(LED_BUILTIN); // Works also for ESP32: static const uint8_t LED_BUILTIN = 8; #define LED_BUILTIN LED_BUILTIN
#endif

#if FLASHEND >= 0x3FFF // For 16k flash or more, like ATtiny1604. Code does not fit in program memory of ATtiny85 etc.
  Serial.println();
#if defined(DEBUG_BUTTON_PIN)
  Serial.print(F("If you connect debug pin "));
  Serial.print(DEBUG_BUTTON_PIN);
  Serial.println(F(" to ground, raw data is always printed and tone is disabled"));
#endif

  // infos for receive
  Serial.print(RECORD_GAP_MICROS);
  Serial.println(F(" us is the (minimum) gap, after which the start of a new IR packet is assumed"));

#if defined(USE_THRESHOLD_DECODER)
  Serial.println(F("Threshold decoding is active and thus MARK_EXCESS_MICROS is set to 0"));
#else
  Serial.print(MARK_EXCESS_MICROS);
  Serial.println(F(" us are subtracted from all marks and added to all spaces for decoding"));
#endif
#endif // FLASHEND >= 0x3FFF

  configureMotorPins();

  Serial.println("L293D V4.x Controller LIVE.");
  Serial.println("Awaiting telemetry...");
}

void loop()
{
  /*
   * Check if received data is available and if yes, try to decode it.
   * Decoded result is in the IrReceiver.decodedIRData structure.
   *
   * E.g. command is in IrReceiver.decodedIRData.command
   * address is in command is in IrReceiver.decodedIRData.address
   * and up to 32 bit raw data in IrReceiver.decodedIRData.decodedRawData
   */
  if (IrReceiver.decode())
  {
    Serial.println();
#if FLASHEND < 0x3FFF // For less than 16k flash, only print a minimal summary of received data
    IrReceiver.printIRResultMinimal(&Serial);
#else

    /*
     *
     */
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_WAS_OVERFLOW)
    {
      handleOverflow();
    }
    else
    {
      /*
       * No overflow here.
       * Stop receiver, generate a single beep, print short info and send usage and start receiver again
       */
      if ((IrReceiver.decodedIRData.protocol != SONY) && (IrReceiver.decodedIRData.protocol != PULSE_WIDTH) && (IrReceiver.decodedIRData.protocol != PULSE_DISTANCE) && (IrReceiver.decodedIRData.protocol != UNKNOWN)
#if defined(DEBUG_BUTTON_PIN)
          && digitalRead(DEBUG_BUTTON_PIN) != LOW
#endif
      )
      {
        /*
         * For SONY the tone prevents the detection of a repeat after the 15 ms SONY gap.
         * In debug mode and for unknown protocols, we need the time for extended output.
         * Skipping tone will get exact gap time between transmissions and not running into repeat frames while wait for tone to end.
         * This in turn enables the next CheckForRecordGapsMicros() call a chance to eventually propose a change of the current RECORD_GAP_MICROS value.
         */
        generateTone();
      }

      /*
       * Print info
       */
      if (IrReceiver.decodedIRData.protocol == UNKNOWN
#if defined(DEBUG_BUTTON_PIN)
          || digitalRead(DEBUG_BUTTON_PIN) == LOW
#endif
      )
      {
        // We have debug enabled or an unknown protocol, print extended info
        if (IrReceiver.decodedIRData.protocol == UNKNOWN)
        {
          Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
        }
        IrReceiver.printIRResultRawFormatted(&Serial, true);
      }
      if (IrReceiver.decodedIRData.protocol == UNKNOWN)
      {
        auto tDecodedRawData = IrReceiver.decodedIRData.decodedRawData; // uint32_t on 8 and 16 bit CPUs and uint64_t on 32 and 64 bit CPUs
        Serial.print(F("Raw data from hash decoder is 0x"));
#if (__INT_WIDTH__ < 32)
        Serial.println(tDecodedRawData);
#else
        PrintULL::println(&Serial, tDecodedRawData, BIN);
#endif
      }
      else
      {
        /*
         * The info output for a successful receive
         */
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial); // Calls printIRResultShort() and other functions, if protocol is UNKNOWN
      }
    }
#endif // #if FLASHEND >= 0x3FFF

    /*
     * !!!Important!!! Enable receiving of the next value, because receiving
     * has stopped after the end of the current received data packet.
     * Do it here, to preserve raw data for printing with printIRResultRawFormatted()
     */
    IrReceiver.resume();

    /*
     * Finally check the received data and perform actions according to the received address and commands
     */

    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)
    {
      Serial.println(F("Repeat received. Here you can repeat the same action as before."));
    }
    else
    {
      if (IrReceiver.decodedIRData.address == 0)
      {
        uint16_t command = IrReceiver.decodedIRData.command;
        Serial.print("Received Command: 0x");
        Serial.print(command, HEX);

         executeCommand(command);
      }
    }

    // Check if repeats of the IR command was sent for more than 1000 ms
    if (detectLongPress(1000))
    {
      Serial.print(F("Command 0x"));
      Serial.print(IrReceiver.decodedIRData.command, HEX);
      Serial.println(F(" was repeated for more than 2 seconds"));
    }
  } // if (IrReceiver.decode())

  /*
   * Your code here
   * For all users of the FastLed library, use this code for strip.show() to improve receiving performance (which is still not 100%):
   * if (IrReceiver.isIdle()) {
   *     strip.show();
   * }
   */
}

#if FLASHEND >= 0x3FFF // No tone() available when using ATTinyCore
/*
 * Stop receiver, generate a single beep and start receiver again
 */
void generateTone()
{
#if !defined(ESP8266) && !defined(NRF5) && defined(TONE_PIN) // tone on esp8266 works only once, then it disables IrReceiver.restartTimer() / timerConfigForReceive().
#if defined(ESP32)                                           // ESP32 uses another timer for tone(), maybe other platforms (not tested yet) too.
  tone(TONE_PIN, 2200, 8);
#else
  IrReceiver.stopTimer(); // Stop timer consistently before calling tone() or other functions using the timer resource.
  tone(TONE_PIN, 2200, 8);
  delay(8);
  IrReceiver.restartTimer(); // Restart IR timer after timer resource is no longer blocked.
#endif
#endif
}
#endif // FLASHEND >= 0x3FFF

void handleOverflow()
{
  Serial.println(F("Overflow detected"));
  Serial.println(F("Try to increase the \"RAW_BUFFER_LENGTH\" value of " STR(RAW_BUFFER_LENGTH) " in " __FILE__));
  // see also https://github.com/Arduino-IRremote/Arduino-IRremote#compile-options--macros-for-this-library

#if !defined(ESP8266) && !defined(NRF5) && FLASHEND >= 0x3FFF && defined(TONE_PIN) // tone on esp8266 works once, then it disables IrReceiver.restartTimer() / timerConfigForReceive().
                                                                                   /*
                                                                                    * Stop timer, generate a double beep and start timer again
                                                                                    */
#if defined(ESP32)                                                                 // ESP32 uses another timer for tone()
  tone(TONE_PIN, 1100, 10);
  delay(50);
  tone(TONE_PIN, 1100, 10);
#else
  IrReceiver.stopTimer();
  tone(TONE_PIN, 1100, 10);
  delay(50);
  tone(TONE_PIN, 1100, 10);
  delay(50);
  IrReceiver.restartTimer();
#endif
#endif
}

unsigned long sMillisOfFirstReceive;
bool sLongPressJustDetected;
/**
 * True once we received the consecutive repeats for more than aLongPressDurationMillis milliseconds.
 * The first frame, which is no repeat, is NOT counted for the duration!
 * @return true once after the repeated IR command was received for longer than aLongPressDurationMillis milliseconds, false otherwise.
 */
bool detectLongPress(uint16_t aLongPressDurationMillis)
{
  if (!sLongPressJustDetected && (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT))
  {
    /*
     * Here the repeat flag is set (which implies, that command is the same as the previous one)
     */
    if (millis() - aLongPressDurationMillis > sMillisOfFirstReceive)
    {
      sLongPressJustDetected = true; // Long press here
    }
  }
  else
  {
    // No repeat here
    sMillisOfFirstReceive = millis();
    sLongPressJustDetected = false;
  }
  return sLongPressJustDetected; // No long press here
}

void executeCommand(uint16_t cmd) {
  switch (cmd) {
    case CMD_UP: 
      Serial.println(" -> Command: UP. Direction set FORWARD.");
      isForward = true;
      isRunning = true;
      spinForward();
      break;
      
    case CMD_DOWN: 
      Serial.println(" -> Command: DOWN. Direction set REVERSE.");
      isForward = false;
      isRunning = true;
      spinReverse();
      break;
      
    case CMD_STOP: 
      Serial.println(" -> Command: STOP. Applying BRAKE.");
      isRunning = false;
      stopEngine();
      break;

    case CMD_POWER:
    case CMD_PAUSE:
      // The Toggle Logic: Invert the current running state
      isRunning = !isRunning; 
      
      Serial.print(" -> Command: TOGGLE. ");
      if (isRunning) {
        Serial.println(isForward ? "Resuming FORWARD." : "Resuming REVERSE.");
      } else {
        Serial.println("Applying BRAKE.");
      }
      applyMotorState();
      break;
      
    default:
      Serial.println(" -> Command unmapped. Ignoring.");
      break;
  }
}

void applyMotorState() {
  if (!isRunning) {
    stopEngine();
  } else if (isForward) {
    spinForward();
  } else {
    spinReverse();
  }
}

void configureMotorPins() {
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  digitalWrite(MOTOR_IN1_PIN, LOW);
  digitalWrite(MOTOR_IN2_PIN, LOW);
}

void stopEngine() {
  digitalWrite(MOTOR_IN1_PIN, LOW);
  digitalWrite(MOTOR_IN2_PIN, LOW);
}

void spinForward() {
  digitalWrite(MOTOR_IN1_PIN, HIGH);
  digitalWrite(MOTOR_IN2_PIN, LOW);
}

void spinReverse() {
  digitalWrite(MOTOR_IN1_PIN, LOW);
  digitalWrite(MOTOR_IN2_PIN, HIGH);
}
