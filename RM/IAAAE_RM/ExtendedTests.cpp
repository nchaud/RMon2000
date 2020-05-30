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

	/******* SUB-BYTE TESTS *********/
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
	
	//Print test
	RM_LOG(F("Test For Print Output:"));
	Helpers::printRSSI(&result);
	RM_LOGLN(F("--------------------------"));
	
	/*************************/
	
	
	/******* ENCODING TESTS *********/
	
	
	
	char p = 250;
	uint8_t val22 = (uint8_t)p;
	RM_LOG2(F("CHAR EXCEEDED BOUNDS IS"), val22);
	
	
	
	
	
	
	
	//1) Single round-trip test
	
	SensorData sd;			//size ~ 10 bytes
	sd.battVoltage = 20245; //Includes mV - e.g. 20.245V
	sd.current = 65535;
	sd.errorChar = 3;
	sd.pVVoltage = 64913;
	sd.temperature = 0;
	Helpers::printSensorData(&sd);
	
	uint8_t typicalMemUsage = sizeof("20245-65535-3-64913-0"); //23 bytes
	RM_LOG2("Basic int->str usage would be", typicalMemUsage);
	
	char output[100];
	int len = Helpers::base64_encode(output, (char*)&sd, sizeof(SensorData));
	RM_LOG("Encoded result to be sent over Web is ");
	RM_LOG(output);
	RM_LOG2(", with size of ", len);
	RM_LOGLN("\t(excluding the '0' at the end incase to be treated as string)");
	
	
	SensorData sdAfter;
	int lenAfter = Helpers::base64_decode((char*)&sdAfter, output, len);
	RM_LOG2("Decoded result received over Web has size of ", lenAfter);
	Helpers::printSensorData(&sdAfter);
	
	if (sdAfter.battVoltage != sd.battVoltage) RM_LOGLN(F("*** ENC TEST FAIL @1 ***"));
	if (sdAfter.current != sd.current) RM_LOGLN(F("*** ENC TEST FAIL @2 ***"));
	if (sdAfter.errorChar != sd.errorChar) RM_LOGLN(F("*** ENC TEST FAIL @3 ***"));
	if (sdAfter.pVVoltage != sd.pVVoltage) RM_LOGLN(F("*** ENC TEST FAIL @4 ***"));
	if (sdAfter.temperature != sd.temperature) RM_LOGLN(F("*** ENC TEST FAIL @5 ***"));
	
	RM_LOGLN(F("--------------------------"));
	
	//2) Test a large sequence of them to ensure correctness- MAX_READINGS constant?
	
	uint8_t COUNT = 20;
	SensorData bulkSd[COUNT];
	for(uint8_t i=0;i<COUNT;i++){
		
		SensorData iSd = bulkSd[i];
		iSd.battVoltage = i;
		iSd.current = i*10;
		iSd.pVVoltage = i*100;
		iSd.temperature = i+50;
	}
	
	FONA_GET_RSSI rssi;
	rssi.rssi = 15;
	rssi.ber = 3;
	rssi.netReg = (FONA_GET_NETREG)(FONA_GET_NETREG::NETSTAT_4 | FONA_GET_NETREG::RESULT_CODE_1);
	
	GsmPayload gsm;
	gsm.moduleId=33;
	gsm.thisBootNumber = 1055;
	gsm.rssi = rssi;
	gsm.addSensorData(&bulkSd[0], COUNT);
	
	char forWeb[1000];
	gsm.getPayload(&forWeb[0], 1000);
	
	RM_LOG(F("GSM Payload To Be Sent Over Web:"));
	RM_LOGLN(forWeb);
	
	RM_LOGLN(F("------------------------"));
	/*************************/
	
	
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
	
	
	//TODO: This test gets passed signal data, doesn't fetch from mem itself OR call sendData() in main?
	//TODO: Another test to check compression of data, not to do in this test
	
	
	
	
	
        // Post data to website
        uint16_t statuscode;
        int16_t length;
        char url[80] = "httpbin.org/post"; url[16]=0; //end
        char data[80] = "hello"; url[5]=0; //end

        Serial.println(F("****"));
        if (!fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data,
								  strlen(data), &statuscode, (uint16_t *)&length)) {
									  
	        Serial.println("Failed!");
        }
        while (length > 0) {
	        while (fona.available()) {
		        char c = fona.read();

				loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
				UDR0 = c;

		        length--;
		        if (! length) break;
	        }
        }
        fona.HTTP_POST_end();
		
		
		
		
			
	
#else
	RM_LOGLN(F("*** FAIL EGT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}
