# Arduino

## Overview

This repository contains the Arduino sketches I've used while experimenting with the [Elegoo UNO Project Super Starter Kit.](https://www.amazon.ca/dp/B01D8KOZF4)

## Remote Flashing UNO

I have my UNO R3 attached to a breadboard on my workbench (aka my dinging room table). Since its usually wired into a project, I don't want to have to bring my laptop over to my workbench, or to disconnect the UNO to bring it to my computer.

Since I have recently added a Raspberry PI 5 to my DIY kit, I keep the Raspberry PI on the side of my workbench, and connect the UNO to the Raspberry PI. I have my Raspberry PI configured as a headless server, and I connect to it over SSH from my laptop, using an SSH key. I use the Arduino CLI from my Raspberry PI to upload sketches to the UNO R3.

## Installing Arduino CLI

Setting up the Arduino CLI was painless, using the [install script](https://github.com/arduino/arduino-cli/blob/master/install.sh) from the [arduino-cli repository](https://github.com/arduino/arduino-cli).

```
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh
```

This creates a bin folder right in the home directory. This new bin folder needs to be added to the PATH. I added `export PATH="$PATH:$/bin"` to `~/.bashrc`, then ran `source ~/.bashrc` to apply the changes.

## Setting up the UNO R3 Board with Arduino CLI

After plugging in the UNO R3 into my Raspberry PI, I ran `arduino-cli board list`. This installed some dependencies and then gave an error saying that there was a missing platform for my Arduino board. To resolve that issue I ran `arduino-cli core install arduino:avr`

Once the correct platform was installed I was able to run `arduino-cli board list` and I saw that my UNO R3 was in the list.

To get specific details on my UNO R3, I ran `arduino-cli board details -b arduino:avr:uno`

```
$ arduino-cli board details -b arduino:avr:uno
Board name:                Arduino UNO
FQBN:                      arduino:avr:uno
Board version:             1.8.7

Official Arduino board:    ✔

Identification properties: pid=0x0043
                           vid=0x2341

Identification properties: pid=0x0001
                           vid=0x2341

Identification properties: pid=0x0043
                           vid=0x2A03

Identification properties: pid=0x0243
                           vid=0x2341

Identification properties: pid=0x006A
                           vid=0x2341

Identification properties: board=uno

Package name:              arduino
Package maintainer:        Arduino
Package URL:               https://downloads.arduino.cc/packages/package_index.tar.bz2
Package website:           https://www.arduino.cc/
Package online help:       https://www.arduino.cc/en/Reference/HomePage

Platform name:             Arduino AVR Boards
Platform category:         Arduino
Platform architecture:     avr
Platform URL:              https://downloads.arduino.cc/cores/staging/avr-1.8.7.tar.bz2
Platform file name:        avr-1.8.7.tar.bz2
Platform size (bytes):     7161883
Platform checksum:         SHA-256:6ae7000b1b6c004c4a208d6d924a8046b417580e4a10cfd3f88930b6660a51db

Required tool: arduino:arduinoOTA     1.3.0
Required tool: arduino:avr-gcc        7.3.0-atmel3.6.1-arduino7
Required tool: arduino:avrdude        8.0.0-arduino1

Programmers:   ID                     Name
               arduinoasisp           Arduino as ISP
               arduinoasispatmega32u4 Arduino as ISP (ATmega32U4)
               arduinoisp             ArduinoISP
               arduinoisporg          ArduinoISP.org
               atmel_ice              Atmel-ICE (AVR)
               avrisp                 AVR ISP
               avrispmkii             AVRISP mkII
               buspirate              BusPirate as ISP
               jtag3                  Atmel JTAGICE3 (JTAG mode)
               jtag3isp               Atmel JTAGICE3 (ISP mode)
               parallel               Parallel Programmer
               stk500                 Atmel STK500 development board
               usbGemma               Arduino Gemma
               usbasp                 USBasp
               usbtinyisp             USBtinyISP
```

## Creating Sketches

To create a new sketch called HelloWorld, I ran `arduino-cli sketch new HelloWorld`. This created a folder called HelloWorld containing an sketch file HelloWorld.ino. The sketch that's created contains the setup and loop functions that is required of every sketch.

```
void setup() {
}

void loop() {
}
```

My editor of choice on Linux is vim. I edited the HelloWorld.ino file and pasted in this code.

```
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
```

## Compiling and Uploading Sketches

The arduino-cli has separate commands for compiling and uploading sketches.

```
arduino-cli compile --fqbn arduino:avr:uno HelloWorld
```

```
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno HelloWorld
```

The arduino-cli also allows for performing both operations in a single command

```
arduino-cli compile --fqbn arduino:avr:uno --port /dev/ttyACM0 --upload HelloWorld
```

## Working with Libraries

The UNO starter kit I purchased had a download link that contained samples that included libraries. I found out the hard way that these libraries and examples are out of date and when the arduino-cli updates libraries, there can be breaking changes.

I experimented with the starter kit and examples directly on my laptop using the Arduino IDE and manually adding libraries that was included with the examples from the kit. Moving to the Raspberry PI remote flashing using the arduino-cli is a much better experience, IMO.

Arduino maintains a list of libraries that people submit for working with Arduino modules. The arduino-cli has commands for searching, installing and updating these libraries.

For example, I was experimenting with the INA219 Current Sensor module and I needed a library. I ran `rduino-cli lib search ina219` to search for matching libraries. There were many different versions available, and it wasn't immediately obvious which one I should choose. I Googled these libraries adn eventually decided to go with the Adafruit version. I installed that library using the command `arduino-cli lib install "Adafruit INA219"`.

## Viewing the Serial Console

To view the serial output from the UNO R3, I used the `arduino-cli monitor` command. This is similar to the Serial Monitor in the Arduino IDE. I ran `arduino-cli monitor -p /dev/ttyACM0 --config baudrate=9600` to view the serial output from the UNO R3 at a baud rate of 9600.
