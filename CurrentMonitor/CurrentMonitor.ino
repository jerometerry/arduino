#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 ina219;

void setup(void)
{
	Serial.begin(115200);
	while (!Serial)
	{
		delay(1);
	} // Wait for serial console

	if (!ina219.begin())
	{
		Serial.println("Failed to find INA219 chip");
		while (1)
		{
			delay(10);
		}
	}

	Serial.println("INA219 Connected. Measuring L6 Engine Load...");
}

void loop(void)
{
	float busV = ina219.getBusVoltage_V();
	float current = ina219.getCurrent_mA();

	Serial.print(busV);
	Serial.print(",");
	Serial.println(current);

	delay(50);
}