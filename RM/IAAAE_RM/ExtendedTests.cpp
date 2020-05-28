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


//RMonV3 types/flags test
void ExtendedTests::runExtendedTypesTest() {
	
#if IS_EXTENDED_TYPES_TEST == true

	FONA_GET_NETREG regVal = (FONA_GET_NETREG)
							(FONA_GET_NETREG::NETSTAT_5 |
							FONA_GET_NETREG::RESULT_CODE_1 |
							FONA_GET_NETREG::IS_ERROR);
	
	FONA_GET_RSSI result;
	result.netReg = regVal;
					 
	RM_LOG(F("Netstat 5 + ResultCode 1 + Error 1="));
	RM_LOGLNFMT(result.netReg, BIN);

	uint8_t memStoredVal = result.netReg;
	FONA_GET_NETREG back = (FONA_GET_NETREG)memStoredVal;
	RM_LOG(F("Cast back after int storage val:"));
	RM_LOGLNFMT(back, BIN);
	
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_0) RM_LOGLN(F("*** TEST FAIL @1 ***"));
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_1) RM_LOGLN(F("*** TEST FAIL @2 ***"));
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_2) RM_LOGLN(F("*** TEST FAIL @3 ***"));
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_3) RM_LOGLN(F("*** TEST FAIL @4 ***"));
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_4) RM_LOGLN(F("*** TEST FAIL @5 ***"));
	if (NETREG_ONLY_NETSTAT(back) != FONA_GET_NETREG::NETSTAT_5) RM_LOGLN(F("*** TEST FAIL @6 ***"));
	
	if (NETREG_ONLY_RESULT_CODE(back) == FONA_GET_NETREG::RESULT_CODE_0) RM_LOGLN(F("*** TEST FAIL @7 ***"));
	if (NETREG_ONLY_RESULT_CODE(back) != FONA_GET_NETREG::RESULT_CODE_1) RM_LOGLN(F("*** TEST FAIL @8 ***"));
	if (NETREG_ONLY_RESULT_CODE(back) == FONA_GET_NETREG::RESULT_CODE_2) RM_LOGLN(F("*** TEST FAIL @9 ***"));
	
	if (NETREG_ONLY_ERROR(back) != FONA_GET_NETREG::IS_ERROR) RM_LOGLN(F("*** TEST FAIL @10 ***"));
	
	//Print test:
	RM_LOG(F("Print Output:"));
	Helpers::printRSSI(&result);
#else
RM_LOGLN(F("*** FAIL ETT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}



//SMS/GPRS test
void ExtendedTests::runExtendedGsmTest(Adafruit_FONA fona) {
	
#if IS_EXTENDED_GSM_TEST == true

	delay(5000);
	
	RM_LOGLN(F("TEST: Checking rssi..."));
	
	FONA_GET_RSSI result = fona.getRSSI();
	Helpers::printRSSI(&result);	
	
#else
	RM_LOGLN(F("*** FAIL EGT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}
