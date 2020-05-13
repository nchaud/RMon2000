#ifndef __SENSORMANAGER_H__
#define __SENSORMANAGER_H__

#include "DataTypes.h"

class SensorManager
{
private:
	boolean _isMock;
	float readVcc();

public:
	SensorManager(boolean isMock);
	SensorData readData();
	uint16_t takeSampleAnalog(uint8_t pinNo);

};

#endif //__SENSORMANAGER_H__
