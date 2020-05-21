/*
// TODO: Do below APN settings !!
Note that if you need to set a GPRS APN, username, and password scroll down to
the commented section below at the end of the setup() function.
// TODO: Keep on for charging battery at times
*/

#include <Arduino.h>
#include "DataTypes.h"
#include "Helpers.h"
#include "Timing.h"
#include "GsmManager.h"
#include "GpsManager.h"
#include "RmMemManager.h"
#include "SensorManager.h"
#include "ExtendedTests.h"


//C++ instances
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
RmMemManager mem = RmMemManager(false);
GpsManager gps = GpsManager(IS_GPS_MOCK);
GsmManager gsm = GsmManager(IS_GSM_MOCK);
SensorManager sensorMgr = SensorManager(true);

void switchOffSystem();
void on3MinutesElapsed(bool doWrite);
void printData();
void initSubsystems();

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



	delay(3000); //time for hardware peripherals to warm up + for user's serial monitor to connect
	
	
	
	//Turn off redundant Arduino board notification LED controlled by pin 13
	pinMode(13, OUTPUT);
	
	#ifdef OUTPUT_DEBUG
		Serial.begin(9600); //Writes to Serial output
	#endif
	
	RM_LOGLN(F("Starting..."));
	
	initSubsystems();
	
	if (IS_BASIC_MEM_TEST) {
		
		mem.verifyBasicEepRom();
		
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
	
	if (IS_EXTENDED_DUMP_OUTPUT) {
		
		mem.runExtendedDumpOutput();
		
		switchOffSystem();
		return;
	}

	if (INITIALISE_MODULE_ID) {
		
		mem.initialiseModule(INITIALISE_MODULE_ID);
		
		switchOffSystem();
		return;
	}
		
	uint16_t currBootCount = mem.incrementBootCount();
	RM_LOG2(F("Boot Count"), currBootCount);
	
	//Take reading every 5 hours so it's a scattered time reading throughout the week
	_behaviour |= SYS_BEHAVIOUR::TakeReadings;
	
	//Send to HQ every 20 hours
	if (currBootCount > 0 && currBootCount%4 == 0) { //TODO: Overflow?
		
		_behaviour |= SYS_BEHAVIOUR::SendData;
	}
}

void initSubsystems() {

	gps.setFona(fona);
	gsm.setFona(fona);

	if (!gsm.begin()) {
	
		//FONA library did not begin - store in ROM, terminate and don't consume power
		//(TODO + Why would this ever happen?)
		return;
	}
	
	if (!gps.toggleGps(true)) {
	
		//TODO: store in ROM
		return;
	}
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
	
	SensorData sd;
	sensorMgr.readData(&sd);
	
	return true;
}

boolean sendData() {
	
	RM_LOGLN(F("Sending data..."));
	
	return false;
}

//Loop-scoped variables
volatile int _timerCounter = 0;
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
