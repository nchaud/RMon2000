#ifndef __RMMEMMANAGER_H__
#define __RMMEMMANAGER_H__

#include "DataTypes.h"

class RmMemManager
{
//variables
public:
protected:
private:
boolean _isMock;

//functions
public:
	RmMemManager(boolean isMock);
	
	void reset();
	
	//unsigned int getUIntFromMemory(unsigned int address);
	//void setUIntToMemory(unsigned int address, unsigned int value);
	
	unsigned long getLongFromMemory(unsigned int address);
	void setLongToMemory(unsigned int address, unsigned long value);
	
	void appendSensorEntry(SensorData* r);
	unsigned long loadSensorData(SensorData* buffer, unsigned int maxNoOfReadings,
								unsigned long* loadedUpTo); //byte* outputData, unsigned int outputDataMaxLength)

	/* Indicates that data up to this value has been sent and need not be 
	   sent again next time */
	void markDataSent(unsigned long sentUpTo);

	void appendDailyEntry(DailyCycleData* r);
	
	//Called at regular intervals at a fast-rate to toggle LEDs between off-on
	void flashLED();

	//Request to change the state of an LED
	void toggleLED(LED_SEL led_num, LED_STATE state);
	
	float takeSampleAnalog(int pinNo);
protected:
private:

	float readVcc();
}; //RmMemManager

#endif //__RMMEMMANAGER_H__
