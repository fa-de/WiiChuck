#ifndef WiiChuck_h
#define WiiChuck_h

class WiiChuck
{
private:
	int i;
	uint8_t zeroJoyX;	//The center positions
	uint8_t zeroJoyY;
	int angles[3];		//The raw accelerometer data TODO: Expose

public:
	uint8_t joyX;	//the RAW x position of the joystick
	uint8_t joyY;	//				"

	bool buttonZ, buttonC;		//Current ...
	bool previousZ, previousC;	//and previous state of the buttons Z and C

	void begin();			//Call this before any other call to the library
	void calibrateJoy();	//Call this to reset the joystick's center position to the current position
	void update();			//Update data from NunChuck

	int getJoyX();			//Use these to get the calibrated position
	int getJoyY();			//					"
};

#endif
