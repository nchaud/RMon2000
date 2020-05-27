#include "ExtendedTests.h"

ExtendedTests::ExtendedTests() { }

//Bulk read/write signals test
void ExtendedTests::runExtendedMemTest(RmMemManager mem, SensorManager sensorMgr) {
	
#if IS_EXTENDED_MEM_TEST == true

	SensorData sData;
	
	RM_LOGLN(F("TEST: Taking readings..."));
	sensorMgr.readData(&sData);
	
	RM_LOGLN(F("TEST: Appending to Memory..."));
	mem.appendSensorEntry(&sData);
	
	//TODO: Verify mock values match
	
#else
	RM_LOGLN(F("*** FAIL EMT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}


//SMS/GPRS test
void ExtendedTests::runExtendedGsmTest(Adafruit_FONA fona) {
	
#if IS_EXTENDED_GSM_TEST == true

	//TODO: Wait a while?
	
	RM_LOGLN(F("TEST: Checking rssi..."));
	
	uint8_t result = fona.getRSSI();
	RM_LOGLN(result);
	
	
#else
	RM_LOGLN(F("*** FAIL EGT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}
