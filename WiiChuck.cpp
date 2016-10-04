#include "Arduino.h"
#include <Wire.h>

#include "initializer_list.h"

#include "WiiChuck.h"

#define NUNCHUCKADDR 0x52
#define VERYLONG 1000000		//How long to wait after encoutering a critical error

// these may need to be adjusted for each nunchuck for calibration
#define ZEROX 510  
#define ZEROY 490
#define ZEROZ 460
#define RADIUS 210  // probably pretty universal

inline void send(const std::initializer_list<uint8_t> data)
{
	Wire.beginTransmission(NUNCHUCKADDR);
	for (uint8_t d : data)
	{
		Wire.write(d);
	}
	int result = Wire.endTransmission();
	if (result != 0)
	{
		Serial.print("Error in serial comm: "); Serial.println(result);
		delay(VERYLONG);
	}
}

static void dump_memory()
{
	for (uint8_t i = 0; i < 16; i++)
	{
		send({ i * 16 });
		Serial.print(i * 16, HEX);
		Serial.print(": ");
		delay(10);
		Wire.requestFrom(NUNCHUCKADDR, 16); // request data from nunchuck
		while (!Wire.available());
		while (Wire.available())
		{
			Serial.print(Wire.read(), HEX);
			Serial.print(" ");
		}
		Serial.print("\n");
	}
}

void WiiChuck::begin()
{
	Wire.begin();

#ifdef _DEBUG
	//dump_memory();
#endif
	//Send "handshake" for unencrypted communication
	send({ 0xf0, 0x55 });
	delay(1);
	send({ 0xFB, 0x00 });
	delay(1);

	// read the extension type from the register block        
	send({ 0xFC });

	delay(1);

	const int identifier_size = 4;
	char ctrlr_type[identifier_size];
	const uint8_t ctrlr_type_ref[identifier_size] = { 0xa4, 0x20, 0x00, 0x00 }; //Identifier for nunchuck (0xa4 20 01 01 for classic controller?)
	int cnt;
	Wire.requestFrom(NUNCHUCKADDR, identifier_size);               // request data from controller
	for (int cnt = 0; cnt < identifier_size; cnt++)
	{
		if (Wire.available())
		{
			ctrlr_type[cnt] = Wire.read();
		}
	}

	//Check if type is valid
	if (memcmp(ctrlr_type, ctrlr_type_ref, identifier_size) != 0)
	{
		Serial.print("Wrong or no nunchuck detected.");
		for (int cnt = 0; cnt < identifier_size; cnt++)
		{
			Serial.print((uint8_t)ctrlr_type[cnt], HEX);
			Serial.print(" ");
		}
		Serial.println("<");
		delay(1000000); //Sleep
	}
	/*
	//delay(1);
	//send_crypto_key();
	*/
	delay(1);

	//Request current state for next update()
	send({ 0x00 });

	delay(1);
	//Set defaults
	for (i = 0; i < 3; i++) angles[i] = 0;
	zeroJoyX = 127;
	zeroJoyY = 127;
}

void WiiChuck::calibrateJoy()
{
	update();
	zeroJoyX = joyX;
	zeroJoyY = joyY;
}

void WiiChuck::update()
{
	Wire.requestFrom(NUNCHUCKADDR, 6); // request data from nunchuck

	uint8_t status[6];

	int cnt = 0;
	while (Wire.available())
	{
		// receive byte as an integer
		status[cnt] = Wire.read(); //TODO: Decode...
		cnt++;
	}
	if (cnt == 0)
	{
		return;
	}
	else if (cnt == 6)
	{
		previousZ = buttonZ;
		previousC = buttonC;

		joyX = (status[0]);
		joyY = (status[1]);

		for (i = 0; i < 3; i++)
			angles[i] = (status[i + 2] << 2) + ((status[5] & (B00000011 << ((i + 1) * 2)) >> ((i + 1) * 2)));

		buttonZ = !(status[5] & B00000001);
		buttonC = !((status[5] & B00000010) >> 1);
		send({ 0x00 }); // send the request for next update()
	}
	else
	{
		//assert(0);
	}
}

int WiiChuck::getJoyX()
{
	return (int)joyX - zeroJoyX;
}

int WiiChuck::getJoyY()
{
	return (int)joyY - zeroJoyY;
}

static void send_crypto_key()
{
	// send the crypto key (zeros), in 3 blocks of 6, 6 & 4.
	send({ 0xF0, 0xAA }); // {crypto key command register, sends crypto enable notice}
	delay(1);
	send({ 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
	delay(1);
	send({ 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 });
	delay(1);
	send({ 0x40, 0x00, 0x00, 0x00, 0x00 });
}
