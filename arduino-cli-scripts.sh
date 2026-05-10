# Install Arduino CLI on Linux
curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | sh

# Arduino CLI creates a bin folder in your home directory that needs to be added to your PATH environment variable. You can do this by adding the following line to your shell configuration file (e.g., .bashrc, .zshrc):
# export PATH="$PATH:$/bin" 

# After adding the line, reload your shell configuration by running:
source ~/.bashrc

# display version info
arduino-cli version

# List all boards connected to your computer
# This will install the necessary dependencies
arduino-cli board list

# Install the arduino:avr platform to communicate with boards such as UNO R3 
arduino-cli core install arduino:avr

# Display information about an Arduino board, such as the UNO R3
arduino-cli board details -b arduino:avr:uno

# Search for libraries
arduino-cli lib search ina219

@ Install library
arduino-cli lib install "Adafruit INA219"

# Create a new sketch named "HelloWorld" in the current directory
# Will create HelloWorld/HelloWorld.ino with empty setup() and loop() functions
arduino-cli sketch new HelloWorld

# Compile the "HelloWorld" sketch for the Arduino UNO R3 board
arduino-cli compile --fqbn arduino:avr:uno HelloWorld

# Flash the "HelloWorld" sketch to the Arduino UNO R3 board connected to /dev/ttyACM0
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno HelloWorld

# Compile and upload the "HelloWorld" sketch to the Arduino UNO R3 board in one step
arduino-cli compile --fqbn arduino:avr:uno --port /dev/ttyACM0 --upload HelloWorld

# Monitor the serial output from the Arduino UNO R3 board connected to /dev/ttyACM0 with a baud rate of 9600
arduino-cli monitor -p /dev/ttyACM0 --config baudrate=9600
