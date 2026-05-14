#include <Wire.h>
#include <Adafruit_INA219.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

Adafruit_INA219 ina219;

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiAvrI2c oled;

void setup(void)
{
	Serial.begin(115200);

	while (!Serial)
	{
		delay(1);
	} // Wait for serial console

#if RST_PIN >= 0
	oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else  // RST_PIN >= 0
	oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

	// Call oled.setI2cClock(frequency) to change from the default frequency.

	oled.setFont(System5x7);
	oled.clear();
	oled.print("Monitoring Current...");

	if (!ina219.begin())
	{
		Serial.println("Failed to find INA219 chip");
		while (1)
		{
			delay(10);
		}
	}

	Serial.println("INA219 Connected. Measuring L6 Engine Load...");
	oled.clear();
	oled.home();

	oled.println("INA219 Connected");
	oled.println("Measuring Current...");
}

void loop(void)
{
	float shuntvoltage = 0;
	float busvoltage = 0;
	float current_mA = 0;
	float loadvoltage = 0;
	float power_mW = 0;

	shuntvoltage = ina219.getShuntVoltage_mV();
	busvoltage = ina219.getBusVoltage_V();
	current_mA = ina219.getCurrent_mA();
	power_mW = ina219.getPower_mW();
	loadvoltage = busvoltage + (shuntvoltage / 1000);

	Serial.print("Bus Voltage:   ");
	Serial.print(busvoltage);
	Serial.println(" V");
	Serial.print("Shunt Voltage: ");
	Serial.print(shuntvoltage);
	Serial.println(" mV");
	Serial.print("Load Voltage:  ");
	Serial.print(loadvoltage);
	Serial.println(" V");
	Serial.print("Current:       ");
	Serial.print(current_mA);
	Serial.println(" mA");
	Serial.print("Power:         ");
	Serial.print(power_mW);
	Serial.println(" mW");
	Serial.println("");

	oled.clear();
	oled.home();

	oled.print("Bus: ");
	oled.print(busvoltage);
	oled.println(" V");
	oled.print("Shunt: ");
	oled.print(shuntvoltage);
	oled.println(" V");
	oled.print("Load: ");
	oled.print(loadvoltage);
	oled.println(" V");
	oled.print("Current: ");
	oled.print(current_mA);
	oled.println(" mA");
	oled.print("Power: ");
	oled.print(power_mW);
	oled.println(" mW");

	delay(2000);
}
