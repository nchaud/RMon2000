#include "ExtendedTests.h"
	
ExtendedTests::ExtendedTests() { }

//Gsm Payload Generation Test: Several signals readings & module data
void ExtendedTests::runExtendedMemTest(RmMemManager mem, SensorManager sensorMgr) {
	
#if IS_EXTENDED_MEM_TEST == true

	SensorData sData;
	
	RM_LOGLN(F("TEST: Taking readings..."));
	sensorMgr.readData(&sData);
	
	RM_LOGLN(F("TEST: Appending to Memory..."));
	mem.appendSensorEntry(&sData);


	
	
	//TODO: Verify mock values match
	//TODO: Test eeprom rolling logic (write to eeprom last-written place (?))
	
	
	
	
	RM_LOGLN(F("~~~~~~~~~~~~~~~ Extended Mem Test Complete~~~~~~~~~~~~~~~~"));
#else
	RM_LOGLN(F("*** FAIL EMT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}


//RMonV3 types/flags test

void writeMockSD(SensorData* iSd, uint8_t i){
	iSd->battVoltage = (i+1);
	iSd->current = (i+1)*10;
	iSd->pVVoltage = (i+1)*100;
	iSd->temperature = (i+1)+50;
	iSd->errorChar = i%5==0?3:0;
}

#if IS_EXTENDED_TYPES_TEST == true

void encodeIntTest(uint8_t i) {
	
	uint8_t input[1];
	input[0] = i;
		
	char output2[10]{0};
	Helpers::base64_encode(&output2[0], (uint8_t*)&input, 1);
		
	uint8_t output3[10]{0};
	Helpers::base64_decode((uint8_t*)&output3, &output2[0], 10);
		
	if ((uint8_t)output3[0] != i) RM_LOGLN(F("*** INT ENC FAIL @1 ***"));
	if ((uint8_t)output3[1] != 0) RM_LOGLN(F("*** INT ENC FAIL @2 ***")); //Overflow
}

void encodeStrTest(char* encVal, char* decVal) {
	
	int inputLen = strlen(encVal);
	int decodeLen = strlen(decVal);
	
	RM_LOG2("Running Test For ", encVal);
	int encLen = Helpers::base64_enc_len(inputLen); //No@terminal char
	if (encLen != strlen(decVal)+1) RM_LOGLN(F("*** STR ENC FAIL @1 ***"));
		
	char strEncoded[20];
	Helpers::fillArray((uint8_t*)strEncoded, sizeof(strEncoded), 1); //Extra buffer of 1s to test no overspill

	int strEncodedLen = Helpers::base64_encode(strEncoded, (uint8_t*)encVal, inputLen);
	
	if (strEncodedLen != strlen(decVal)+1) RM_LOGLN(F("*** STR ENC FAIL @2 ***")); //Yes@terminal char
	if (decVal[decodeLen-1] != strEncoded[decodeLen-1]) RM_LOGLN(F("*** STR ENC FAIL @3 ***"));
	if (0 != strEncoded[decodeLen]) RM_LOGLN(F("*** STR ENC FAIL @4 ***"));
	if (1 != strEncoded[decodeLen+1]) RM_LOGLN(F("*** STR ENC FAIL @5 ***")); //May have overwritten
		
	int expStrDecodingLen = Helpers::base64_dec_len(decVal, decodeLen+1);
	if (expStrDecodingLen != strlen(encVal)) RM_LOGLN(F("*** STR DEC FAIL @1 ***")); //No@terminal char
		
	char strDecoded[20];
	Helpers::fillArray((uint8_t*)strDecoded, sizeof(strDecoded), 1); //Extra buffer of 1s to test no overspill
		
	int strDecodedLen = Helpers::base64_decode((uint8_t*)strDecoded, decVal, decodeLen+1);
	if (strDecodedLen != strlen(encVal)) RM_LOGLN(F("*** STR DEC FAIL @2 ***")); //No@terminal char
	if (encVal[inputLen-1] != strDecoded[inputLen-1]) RM_LOGLN(F("*** STR DEC FAIL @3 ***"));
	if (1 != strDecoded[inputLen]) RM_LOGLN(F("*** STR DEC FAIL @4 ***")); //May have introduced \0, WRONG
}

void encodeSingleSensorTest(){
	
	SensorData sd;			//size ~ 10 bytes
	sd.battVoltage = 20245; //Includes mV - e.g. 20.245V
	sd.current = 65535;
	sd.errorChar = 3;
	sd.pVVoltage = 64913;
	sd.temperature = 0;
	Helpers::printSensorData(&sd);
	
	uint8_t typicalMemUsage = sizeof("20245-65535-3-64913-0"); //23 bytes
	RM_LOG2("Basic int->str usage would be", typicalMemUsage);
	
	// ENCODING
	char output[100]; //For testing, make a large buffer and check it doesn't overspill
	int len = Helpers::base64_encode(output, (uint8_t*)&sd, sizeof(SensorData));
	int expectedLen = Helpers::base64_enc_len(sizeof(SensorData));
	RM_LOG("Encoded result to be sent over Web is ");
	RM_LOGLN(output);
	RM_LOG("\t with size of ");
	RM_LOG(strlen(output)+1);
	RM_LOGLN(" (including terminating '0')");
	if ((strlen(output)+1) != len) RM_LOGLN(F("*** ENC LEN FAIL @1 ***"));
	if ((strlen(output)+1) != expectedLen) RM_LOGLN(F("*** ENC LEN FAIL @2 ***"));
	
	// DECODING
	SensorData sdAfter;
	int expDecodingLen = Helpers::base64_dec_len(output, len);
	int lenAfter = Helpers::base64_decode((uint8_t*)&sdAfter, output, len);
	RM_LOG2("Decoded result received over Web has size of ", lenAfter);
	Helpers::printSensorData(&sdAfter);
	
	if (sizeof(SensorData) != lenAfter) RM_LOGLN(F("*** DEC LEN FAIL @1 ***"));
	if (sizeof(SensorData) != expDecodingLen) RM_LOGLN(F("*** DEC LEN FAIL @2 ***"));
	
	if (sdAfter.battVoltage != sd.battVoltage) RM_LOGLN(F("*** CMP TEST FAIL @1 ***"));
	if (sdAfter.current != sd.current) RM_LOGLN(F("*** CMP TEST FAIL @2 ***"));
	if (sdAfter.errorChar != sd.errorChar) RM_LOGLN(F("*** CMP TEST FAIL @3 ***"));
	if (sdAfter.pVVoltage != sd.pVVoltage) RM_LOGLN(F("*** CMP TEST FAIL @4 ***"));
	if (sdAfter.temperature != sd.temperature) RM_LOGLN(F("*** CMP TEST FAIL @5 ***"));
	if (sdAfter.dataType != sd.dataType) RM_LOGLN(F("*** CMP TEST FAIL @6 ***"));
}

void encodeBulkSignalsTest(char* forWeb, uint8_t COUNT) {
	
	SensorData bulkSd[COUNT];
	for(uint8_t i=0;i<COUNT;i++){
		
		SensorData* iSd = &bulkSd[i];
		writeMockSD(iSd, i);
	}
	
	//	uint8_t* ptrToFirst = (uint8_t*)&bulkSd[0];
	//	RM_LOG2(F(">Batt WAS "), * ((uint16_t*)(ptrToFirst+offsetof(SensorData, battVoltage))));
	
	FONA_GET_RSSI rssi;
	rssi.rssi = 15;
	rssi.ber = 3;
	rssi.netReg = (FONA_GET_NETREG)(FONA_GET_NETREG::NETSTAT_4 | FONA_GET_NETREG::RESULT_CODE_1);
	
	GsmPayload gsm;
	gsm.setModuleId(33);
	gsm.setBootNumber( 1026);
	gsm.setRSSI( rssi);
	gsm.setSensorData(&bulkSd[0], COUNT);
	
	gsm.createEncodedPayload(forWeb);
}
#endif


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
	
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_0) RM_LOGLN(F("*** NETREG FAIL @1 ***"));
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_1) RM_LOGLN(F("*** NETREG FAIL @2 ***"));
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_2) RM_LOGLN(F("*** NETREG FAIL @3 ***"));
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_3) RM_LOGLN(F("*** NETREG FAIL @4 ***"));
	if (NETREG_ONLY_NETSTAT(back) == FONA_GET_NETREG::NETSTAT_4) RM_LOGLN(F("*** NETREG FAIL @5 ***"));
	if (NETREG_ONLY_NETSTAT(back) != FONA_GET_NETREG::NETSTAT_5) RM_LOGLN(F("*** NETREG FAIL @6 ***"));
	
	if (NETREG_ONLY_RESULT_CODE(back) == FONA_GET_NETREG::RESULT_CODE_0) RM_LOGLN(F("*** NETREG FAIL @7 ***"));
	if (NETREG_ONLY_RESULT_CODE(back) != FONA_GET_NETREG::RESULT_CODE_1) RM_LOGLN(F("*** NETREG FAIL @8 ***"));
	if (NETREG_ONLY_RESULT_CODE(back) == FONA_GET_NETREG::RESULT_CODE_2) RM_LOGLN(F("*** NETREG FAIL @9 ***"));
	
	if (NETREG_ONLY_ERROR(back) != FONA_GET_NETREG::IS_ERROR) RM_LOGLN(F("*** NETREG FAIL @10 ***"));
	
	//Print test
	RM_LOG(F("Test For Print Output:"));
	Helpers::printRSSI(&result);
	
	/*********************************/
	
	
	/******* ENCODING TESTS *********/
	
	//	** 1) Ensure this is avoided with lib:- **
	//		char r = -127; char q = 129; Serial.println((uint8_t)q==(uint8_t)r); //This is true in Arduino 
	for(uint8_t i=0 ; ; i++) {
	
		encodeIntTest(i);
		if (i==255)
			break; //max for unsigned byte
	}

	//	** 2) Check encoding/decoding for strings with their null terminating char \0 **
	//		Verify encoding/decoding %3 behaviour works
	encodeStrTest("h", "aA==");
	encodeStrTest("he", "aGU=");
	encodeStrTest("hel", "aGVs");
	encodeStrTest("hell", "aGVsbA==");
	encodeStrTest("hello", "aGVsbG8=");
	
	
	//	** 3) Single sensor-data round-trip numbers test **
	encodeSingleSensorTest();
	
	
	//	** 4) Test a full gsm-payload incl. large sequence of them to ensure correctness **

	//TODO: MAX READINGS A CONSTANT?
	
	//Encoding
	int COUNT=10; //Can do ~25 if only encoding but this much if also decoding in this test
				  //- CHECK RAM STATE HERE AND WITHIN READING
				  //https://playground.arduino.cc/Code/AvailableMemory/
	
	uint16_t encodedSz = GsmPayload::getEncodedPayloadSize_S(COUNT);
	char forWeb[encodedSz];
	encodeBulkSignalsTest(forWeb, COUNT);
	
	RM_LOGLN(F("GSM Payload To Be Sent Over Web:"));
	RM_LOGLN(forWeb);
	//		Helpers::printByteArray((uint8_t*)(&forWeb[0]), 20);
	
	//Decoding
	uint8_t numReadings = GsmPayload::readNumSReadings(forWeb, encodedSz);
	if (numReadings != COUNT) RM_LOGLN(F("*** READ NUM FAIL ***"));

	//Now parse it
	GsmPayload receivedPayload;
	SensorData receivedSensorData[numReadings];
	receivedPayload.readEncodedPayload(forWeb, encodedSz, (SensorData*)&receivedSensorData);
	
	RM_LOGLN(F("First Parsed Reading:"));
	SensorData* readOne = receivedPayload.getSensorData();
	Helpers::printSensorData(readOne); //print the first
	
	for(uint8_t i=0;i<numReadings;i++){
		
		SensorData expectedVal;
		SensorData* expectedValPtr=&expectedVal;
		writeMockSD(expectedValPtr, i);
		
		SensorData* parsed = readOne + i;
		
		if (expectedValPtr->battVoltage != parsed->battVoltage) {RM_LOG2(F("*** BATT FAIL ***"), i);}
		if (expectedValPtr->current != parsed->current) {RM_LOG2(F("*** CURR FAIL ***"), i);}
		if (expectedValPtr->dataType != parsed->dataType) {RM_LOG2(F("*** DT FAIL ***"), i);}
		if (expectedValPtr->errorChar != parsed->errorChar) {RM_LOG2(F("*** ERR FAIL ***"), i);}
		if (expectedValPtr->pVVoltage != parsed->pVVoltage) {RM_LOG2(F("*** PV FAIL ***"), i);}
		if (expectedValPtr->temperature != parsed->temperature) {RM_LOG2(F("*** TEMP FAIL ***"), i);}
	}
	
	/*************************/
	
	RM_LOGLN(F("~~~~~~~~~~~~~~~ Extended Types Test Complete~~~~~~~~~~~~~~~~"));
	
#else
RM_LOGLN(F("*** FAIL ETT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}


//SMS/GPRS test
SensorData* _mockData;

void ExtendedTests::endExtendedGsmTest() {
	
#if IS_EXTENDED_GSM_TEST == true

	//Clear dynamic memory
	free(_mockData);
	
	//Do verification of data test (somehow?)
	
	
	RM_LOGLN(F("~~~~~~~~~~~~~~~ Extended GSM Test Complete~~~~~~~~~~~~~~~~"));
#else
	RM_LOGLN(F("*** FAIL EGT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}

void ExtendedTests::startExtendedGsmTest(RmMemManager* mem) {
	
#if IS_EXTENDED_GSM_TEST == true

    Serial.println(F("****"));
	
	//Malloc as the data will be sent later on when gsm connection made
	int numReadings=12;
	_mockData = (SensorData*)malloc(sizeof(SensorData)*numReadings);
	for(int i=0;i<numReadings;i++)
		writeMockSD(_mockData+i, i);
	
	Serial.println(F("Sensor-Datas going for transmission:"));
	for(int i=0;i<numReadings;i++)
		Helpers::printSensorData(_mockData+i);
	
	mem->mockSensorData = _mockData;
	mem->numMockSensorData = numReadings;
	
#else
	RM_LOGLN(F("*** FAIL EGT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}
