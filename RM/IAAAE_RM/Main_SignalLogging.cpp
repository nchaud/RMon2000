//Testing
#define OUTPUT_DEBUG			//For logging to Serial
#define DEBUG					//Prints output stmts - ONLY WITH LAPTOP - to print output statements whilst all below are running
bool IS_GSM_MOCK = true;		//Without connecting GSM shield
bool IS_GPS_MOCK = true;		//Without connecting GPS shield

//Initialising module
char INIT_MODULE_ID = 5;

bool INITIALISE_MODULE = false;
bool IS_MEM_TEST = false;		//Smoke test new module's EEPROM
bool ONLY_PRINT_DATA = true;	//Analysis/Review afterwards - no writes
uint8_t WRITE_DATA_UPFRONT = 0;	//How many times to write data quickly

//Mock GSM
#ifdef DEBUG
	//Set FONA module to be debug too
	#define ADAFRUIT_FONA_DEBUG
#endif

/*
// TODO: Do below APN settings !!
Note that if you need to set a GPRS APN, username, and password scroll down to
the commented section below at the end of the setup() function.
*/

#include <Arduino.h>
#include "Helpers.h"
#include "DataTypes.h"
#include "Timing.h"
#include "GsmManager.h"
#include "GpsManager.h"
#include "RmMemManager.h"


//C++ instances
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
RmMemManager mem(false);
GpsManager gps(IS_GPS_MOCK);
GsmManager gsm(IS_GSM_MOCK);

void initModule(uint8_t moduleId);
void on3MinutesElapsed(bool doWrite);
void printData();
void initSubsystems();


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

	delay(3000); //time for hardware peripherals to warm up + for user's serial monitor to connect
	
	//Turn off redundant notification LED controlled by pin 13
	pinMode(13, OUTPUT);
	
	#ifdef OUTPUT_DEBUG
		Serial.begin(9600); //Writes to Serial output
	#endif
	
	RM_LOGLN(F("Starting..."));

	initSubsystems();

	if (INITIALISE_MODULE) {
		initModule(INIT_MODULE_ID);
	} else {
		mem.incrementBootCount();
	}
	
	if (WRITE_DATA_UPFRONT>0) {
		
		//TODO !!
		
	}
	
	if (IS_MEM_TEST) {
		mem.verifyEepRom();
	}
	
	if (ONLY_PRINT_DATA) {
		mem.printData();
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
	
	if (!gps.toggleGps(true)){
	
		//TODO: store in ROM
		return;
	}
}

void initModule(uint8_t moduleId) {
	
	mem.initialiseModule(moduleId);

	RM_LOG2("Initialised with id ", moduleId);
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

volatile int _timerCounter = 0;
void loop() {

	++_timerCounter;
		
	delay(1000);
	
	if (ONLY_PRINT_DATA) {
		return;
	}
	
	if (INITIALISE_MODULE) {
		return;
	}

	RM_LOGLN("Looping");
	
	//if (DIAGNOSTIC_TEST) {
	//
		////Write and print every second
		//on3MinutesElapsed(true);
		//printData();
		//return;
	//}

	if (_timerCounter == 3*60)//Run once per startup, at after 3 mins when subsystems toggled&ready
		on3MinutesElapsed(true);
}

/** Timer/power-on testing at intervals
void setup(void)
{
	//Must immediately run
	pinMode(5, OUTPUT);
	digitalWrite(5, HIGH);

	pinMode(4, OUTPUT);

	Serial.begin(9600);
}

int counter = 0;
void loop(){
	
	Serial.write("Loop line");
	
	if (counter%3==0) {
		
		digitalWrite(4, HIGH);
		delay(1000);
		digitalWrite(4, LOW);
	}

	if (counter==10)
	digitalWrite(5, LOW);
	
	delay(1000);
	++counter;
}
*/
