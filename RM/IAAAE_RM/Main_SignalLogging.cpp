/*
// TODO: Do below APN settings !!
Note that if you need to set a GPRS APN, username, and password scroll down to
the commented section below at the end of the setup() function.
// TODO: Keep on for charging battery at times
*/

#include <Arduino.h>
#include "Adafruit_FONA.h"
#include "DataTypes.h"
#include "Helpers.h"
#include "Timing.h"
//#include "GsmManager.h"
//#include "GpsManager.h"
#include "RmMemManager.h"
#include "SensorManager.h"
#include "ExtendedTests.h"


//C++ instances
Adafruit_FONA __fona = Adafruit_FONA(FONA_RST, IS_GSM_MOCK);
RmMemManager mem = RmMemManager(false);
//GpsManager gps = GpsManager(IS_GPS_MOCK);
SensorManager sensorMgr = SensorManager(true);

void switchOffSystem();
void on3MinutesElapsed(bool doWrite);
void printData();
Adafruit_FONA* ensureFonaInitialised(boolean forDataSend, boolean* isPending);

uint8_t _behaviour = SYS_BEHAVIOUR::DoNothing;

void setup() {
	
	//unsigned char ResetSrc = MCUSR;   // TODO: save reset reason if not 0
	//unsigned volatile int px=9;
	//MCUSR = 0x00;  // cleared for next reset detection

	// Optionally configure a GPRS APN, username, and password.
	// You might need to do this to access your network's GPRS/data
	// network.  Contact your provider for the exact APN, username,
	// and password values.  Username and password are optional and
	// can be removed, but APN is required.
	//fona.setGPRSNetworkSettings(F("your APN"), F("your username"), F("your password"));

	// Optionally configure HTTP gets to follow redirects over SSL.
	// Default is not to follow SSL redirects, however if you uncomment
	// the following line then redirects over SSL will be followed.
	//fona.setHTTPSRedirect(true);


	//Must immediately run as this pin in LOW switches off the system
	pinMode(PIN_SHUTDOWN, OUTPUT);
	digitalWrite(PIN_SHUTDOWN, HIGH);



	delay(3000); //time for above to happen + hardware peripherals to warm up + for user's serial monitor to connect
	
	
	
	//Turn off redundant Arduino board notification LED controlled by pin 13
	pinMode(13, OUTPUT);
	
	#ifdef OUTPUT_DEBUG
		Serial.begin(9600); //Writes to Serial output
	#endif
	
	RM_LOGLN(F("Starting..."));
		
	if (IS_BASIC_MEM_TEST) {
		
		mem.verifyBasicEepRom();
		
		switchOffSystem();
		return;
	}

	if (IS_EXTENDED_TYPES_TEST) {
	
		ExtendedTests::runExtendedTypesTest();
	
		switchOffSystem();
		return;
	}
	
	if (IS_EXTENDED_MEM_TEST) {
		
		ExtendedTests::runExtendedMemTest(mem, sensorMgr);
		
		switchOffSystem();
		return;
	}
	
	if (IS_EXTENDED_SHOW_100_BYTES) {
		
		mem.runExtendedShow100Bytes();
		
		switchOffSystem();
		return;
	}
	
	if (IS_EXTENDED_SHOW_MEM) {
		
		mem.runExtendedDumpOutput();
		
		switchOffSystem();
		return;
	}

	if (IS_EXTENDED_GSM_TEST) {
	
		_behaviour |= SYS_BEHAVIOUR::ExtendedGsmTest;
		return;
	}

	if (INITIALISE_MODULE_ID) {
		
		mem.initialiseModule(INITIALISE_MODULE_ID);
		
		switchOffSystem();
		return;
	}
		
	uint16_t currBootCount = mem.incrementBootCount();
	RM_LOG2(F("Boot Count"), currBootCount);
	
	//Take reading every few hours - not a factor of 24 - so it's a scattered time reading throughout the week
	_behaviour |= SYS_BEHAVIOUR::TakeReadings;
	
	//Send to HQ every ~20 hours
	if (currBootCount > 0 && currBootCount%4 == 0) { //TODO: Overflow?
		
		_behaviour |= SYS_BEHAVIOUR::SendData;
	}
}

uint8_t _fonaStatusInit=0;
uint8_t _gprsStatusInit=0;
uint16_t _initFonaLoopCount = 0;
uint16_t _gprsSignalLoopCount = 0;
FONA_GET_RSSI _rssiStatus;
Adafruit_FONA* ensureFonaInitialised(boolean forDataSend, boolean* isComplete) {

	//boolean isFirstLoop = _initFonaLoopCount == 0;
	++_initFonaLoopCount;
	
	*isComplete = true;
	
	if (_fonaStatusInit==0) {
		
		RM_LOGLN(F("Initialising fona..."));
		
		FONA_STATUS_INIT ret = __fona.begin(FONA_TX, FONA_RX);
		_fonaStatusInit = ret;
		
		//TODO: TEST ! with both single-digit module IDs and double digit
		uint8_t moduleId = mem.getModuleId();
		String userAgentStr = "IAAAE_RMonV3_"+moduleId;
		__fona.setUserAgent(userAgentStr);
	}

	if (IS_ERR_FSI(_fonaStatusInit)) {
	
		RM_LOG2(F("Error initialising fona..."), _fonaStatusInit);
	
		//FONA library did not begin - store err in ROM, terminate and don't consume power
		//(TODO + Why would this ever happen?)
		//But don't log it in eeprom if running a test? Basic=non-writing test vs Extended tests?
	
		return NULL;
	}	

	
	if (forDataSend) {
		
		//The first enable attempt will happen after GPRS_ENABLE_INTERVAL
		
		if (_gprsStatusInit != 0) {
			
			//No-op, just return what was calculated the last time
		}
		else if (_initFonaLoopCount > GPRS_MAX_ENABLE_TIME) {
			
			//Just for safety - we should've already calculated the _gprsStatusInit value
			RM_LOGLN(F("__GPRS_OVER_MAX_SHOULD_NEVER_OCCUR__"));
		}
		else if (_initFonaLoopCount % GPRS_ENABLE_INTERVAL != 0) {
			
			//Try to enable it every x seconds for a period
			*isComplete = false;
		}
		else {
			
			RM_LOGLN(F("Attempting to enable GPRS..."));
		
			FONA_STATUS_GPRS_INIT gprsRet = __fona.enableGPRS(true);
		
			if (IS_ERR_FSGI(gprsRet)) {
			
				//TODO: Log this
				//But don't log it in eeprom if running a test? Basic=non-writing test vs Extended tests?
			
				RM_LOG2(F("Error initialising GPRS..."), gprsRet);
				if (_initFonaLoopCount < GPRS_MAX_ENABLE_TIME) {
				
					*isComplete = false;
					RM_LOGLN(F("Will try to enable GPRS again shortly"));
				}
				else {
				
					//We've hit the last interval of trying, so all done trying
					_gprsStatusInit = gprsRet;
					RM_LOGLN(F("All attempts to enable GPRS failed"));
				}
			}
			else {
			
				//Success, we're done initialising GPRS
				_gprsStatusInit = gprsRet;
				RM_LOGLN(F("GPRS initialised successfully !"));
			}
		}
		
		if (_gprsStatusInit != 0) {
			if (IS_ERR_FSGI(_gprsStatusInit)) {
				return NULL;
			}
			else { //Initialised successfully, now ensure good signal
				
				if (Helpers::isSignalGood(&_rssiStatus)) {
					
					//Previously checked - it's fine
				}
				else if (_gprsSignalLoopCount++ > GPRS_MAX_SIGNAL_WAIT_TIME) {
					
					//Wait-time over for signal, try sending regardless of signal value
					RM_LOGLN(F("\t (Good-RSSI Wait Timed Out - will continue regardless now)"));
				}
				else {
					FONA_GET_RSSI rssi = __fona.getRSSI();

					RM_LOG(F("Checking Good-RSSI - currently:"));
					Helpers::printRSSI(&rssi);					
					
					if (Helpers::isSignalGood(&rssi)){
						
						//All done, signal is good now
						RM_LOGLN(F("\t (Good-RSSI - successfull, all done)"));
						_rssiStatus = rssi;
					}
					else{
						
						RM_LOGLN(F("\t (Good-RSSI Failed - will check again after interval)"));
						*isComplete = false;
					}
				}
				
				//Even if we have a bad signal or it times out,
				//	return __fona so they can try sending data anyway
			}
		}
		else {
			return NULL;
		}
	}
	
	return &__fona;
}


void switchOffSystem() {
	
	RM_LOGLN("Switching off...");
	
	digitalWrite(PIN_SHUTDOWN, LOW);
	
	delay(3000); //To allow serial to purge the shutdown message
}

void on3MinutesElapsed(bool doWrite) {

	//RM_LOGLN(F("3 minutes elapsed - logging..."));
	//
	//byte META_SZ = sizeof(ModuleMeta);
	//byte SESSION_SZ = sizeof(SingleSession);
	//
	////Get last reading
	//ModuleMeta meta;
	//readMem(MEM_START, (uint8_t*)&meta, sizeof(ModuleMeta));
	//
	//RM_LOG(F("Module #"));
	//RM_LOG(meta.moduleId);
	//RM_LOG(F(", Current #Readings: "));
	//RM_LOGLN(meta.numReadings);
//
	//if (!doWrite)
		//return;
//
	////Update the number of readings in metadata first so no matter what happens, existing data isnt overwritten
	//meta.numReadings++;
	//writeMem(TART, (uint8_t*)&meta, sizeof(ModuleMeta));
	//
	//SingleSession session;
	//gps.getGpsInfo(session.gpsInfo);
	//gsm.getGsmInfo(session.gsmInfo);
//
	//
	//RM_LOG(F("Got GPS info, lat="));
	//RM_LOG(session.gpsInfo.lat);
	//RM_LOG(F(" lon="));
	//RM_LOG(session.gpsInfo.lon);
	//RM_LOG(F(" date="));
	//RM_LOG(session.gpsInfo.date);
	//RM_LOG(F(" status="));
	//RM_LOG(session.gpsInfo.gpsStatus);
	//RM_LOG(F(" errCode="));
	//RM_LOGLN(session.gpsInfo.errorCode);
	//
	//uint16_t writeAddress = getReadingAddress(meta.numReadings);
////	Serial.print(F("Calculated next address for data to be written to: "));
////	Serial.println(writeAddress);
	//
	////Run 2 tests
	////gsm.setGPRSNetworkSettings
	//String sm = "";//"Module ID:"+ModuleMeta.moduleId+" transmitting.";
	//
	////gsm.sendViaSms(sm.c_str()); //TO: local number !
	////gsm.sendViaGprs(sm.c_str());
	//
	//writeMem(writeAddress, (uint8_t*)&session, SESSION_SZ);
}

boolean takeReadings() {
	
	RM_LOGLN(F("Taking readings..."));
	
	SensorData sd;
	sensorMgr.readData(&sd);
	
	return true;
}

//Adafruit_FONA* _sendDataFona = NULL;
uint16_t _sendDataLoopCount = 0;
boolean sendData() {
	
	boolean isInit = (_sendDataLoopCount == 0);
	
	//Increment before doing any work so doesn't get stuck continuously initialising
	//(by being called from 'loop') due to a loop-resetting error raised by FONA
	++_sendDataLoopCount;
	
	if (isInit)
		RM_LOGLN(F("Initialising Fona to send data"));
	
	boolean isComplete;
	Adafruit_FONA* sendDataFona = ensureFonaInitialised(true, &isComplete);
	
	if (!isComplete) {
		RM_LOGLN(F("\t(Fona Init Pending...)"));
		return false; //Still waiting to initialise
	}
		
	if (sendDataFona == NULL) {
		RM_LOGLN(F("\t(Fona Init ERROR)"));
		return true; //Error initialising
	}
	
	//RM_LOGLN(F("\t(Checking RSSI good enough OR Signal-lock wait time expired)"));
	//
	//FONA_GET_RSSI rssi = sendDataFona->getRSSI();
	//RM_LOG(F("\t(Curr RSSI:)"));
	//Helpers::printRSSI(rssi);
	//
	//boolean fineToSend = rssi.rssiErr == 0 &&
						 //rssi.rssi != 99 && //Not known/Undetectable.
						 //rssi.rssi >= 7;	//@7 RSSI [=100 dBm] (2->30 RSSI = -110dBm -> -54dBm)
			
	//Wait to get signal 
	//	- may already be over the threshold when doing initialisation so kick it off if so
	//	OR should we just check RSSI and send if it's ok?
	if (true) { // _sendDataLoopCount >= GPRS_MAX_SIGNAL_WAIT_TIME) {
		
		//Get RSSI - store? check and/or wait another minute? not?
		FONA_GET_RSSI rssi = sendDataFona->getRSSI();
		//Helpers::printRSSI(&rssi);
		
		SensorData sData[2]; //TODO: HARDCODED
		unsigned long loadedTo;
		mem.loadSensorData((SensorData*)&sData, 2, &loadedTo);

		GsmPayload payload;
		payload.setModuleId(999);
		payload.setBootNumber(33);
		payload.setSensorData((SensorData*)&sData, 2);
		payload.setRSSI(rssi);
		uint16_t encodedSz = GsmPayload::getEncodedPayloadSize_S(2);

		char encodedData[encodedSz];
		payload.createEncodedPayload(encodedData);

		RM_LOGLN(F("Encoded data created and ready for send:"));
		RM_LOGLN(encodedData);

		uint16_t statuscode;
		sendDataFona->sendDataOverGprs((uint8_t*)encodedData, encodedSz, &statuscode);

		//If all done - reset (even though board will be reset - but for tests)
		_sendDataLoopCount = 0;
		
		return true;
	}
	else{
		return false;
	}
}

//Loop-scoped variables
uint16_t _timerCounter = 0;
void loop() {

	delay(1000);
	++_timerCounter;

	RM_LOG2(F("Behaviour"), _behaviour);
	
	if((_behaviour&SYS_BEHAVIOUR::TakeReadings) != 0) {
		
		if (takeReadings())
			_behaviour &= ~SYS_BEHAVIOUR::TakeReadings;
	}
	
	if ((_behaviour&SYS_BEHAVIOUR::SendData) != 0) {
		
		if (sendData())
			_behaviour &= ~SYS_BEHAVIOUR::SendData;
	}

	if ((_behaviour&SYS_BEHAVIOUR::ExtendedGsmTest) != 0) {
	
		if (_timerCounter == 1)
			ExtendedTests::startExtendedGsmTest(&__fona, &mem);
	
		if (sendData()) {
			
			_behaviour &= ~SYS_BEHAVIOUR::ExtendedGsmTest;
			ExtendedTests::endExtendedGsmTest();
		}
	}

	if (_behaviour == SYS_BEHAVIOUR::DoNothing) {
		
		switchOffSystem();
	}
	
	//if (DIAGNOSTIC_TEST) {
	//
		////Write and print every second
		//on3MinutesElapsed(true);
		//printData();
		//return;
	//}

}




//TODO: A good test is putting
//char forWeb[1000]; 
//in the stack, causing a restart and we should get a restart code
//=> to be stored in memory and then module shutdown?






/************************************************************************/
/*             Timer/power-on testing at intervals                      */
/************************************************************************/
/*
void setup() {

	//Must immediately run - reset pin for timer
	pinMode(5, OUTPUT);
	digitalWrite(5, HIGH);

	pinMode(4, OUTPUT); //LED

	pinMode(9, OUTPUT); //buzzer
	
}

int counter = 0;

void loop() {

	if (counter <= 3)
	{
		tone(9, 1000);
		delay(1000);
		noTone(9);
	}

	digitalWrite(4, HIGH);
	delay(1000);
	digitalWrite(4, LOW);

	if (counter==5)
		digitalWrite(5, LOW);

	delay(1000);
	++counter;
}

*/
