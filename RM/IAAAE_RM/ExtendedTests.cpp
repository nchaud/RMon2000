#include <Arduino.h>
#include "DataTypes.h"
#include "ExtendedTests.h"

ExtendedTests::ExtendedTests() { }


//Bulk read/write signals test
void ExtendedTests::runExtendedMemTest(RmMemManager mem, SensorManager sensorMgr) {
	
#if IS_EXTENDED_MEM_TEST == true

	SensorData sData;
	sensorMgr.readData(&sData);
	mem.appendSensorEntry(&sData);
	
	//TODO: Verify mock values match
	
#else
	RM_LOG(F("*** FAIL EMT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}