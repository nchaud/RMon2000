#define DEBUG	  /* Runs all code but debugging it */

//Testing
#define DEBUG 1					//Prints output stsmts - ONLY WITH LAPTOP - to print output statements whilst all below are running
bool DIAGNOSTIC_TEST = true;	//Run 1st - to smoke test new module's EEPROM
bool IS_GSM_MOCK = true;		//Without connecting GSM shield
bool IS_GPS_MOCK = true;		//Without connecting GPS shield

//Initialising module
bool INITIALISE_MODULE = false;
char INIT_MODULE_ID = 5;

//Analysis/Review afterwards - just show the existing data, no writes
bool ONLY_PRINT_DATA = true;

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
#include <Wire.h>
#include "Helpers.h"
#include "DataTypes.h"
#include "Timing.h"
#include "GsmManager.h"
#include "GpsManager.h"
#include "avr/eeprom.h"

//Have seen address 0 is typically worn out from testing in EEPROM and gives bogus reads at times 
//so start higher up on another cell, which is more reliable
uint16_t MEM_START = 170;

//C++ instances
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
//RmMemManager mem(IS_SENSOR_MOCK);
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
	
	Wire.begin();
	
	#ifdef DEBUG
		Serial.begin(9600); //Writes to Serial output
		Serial.println(F("Starting..."));
	#endif

	gps.setFona(fona);
	gsm.setFona(fona);

	if (INITIALISE_MODULE) {
		initModule(INIT_MODULE_ID);
	}

	initSubsystems();
}

void initSubsystems(){

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

uint16_t getReadingAddress(uint8_t readingNum){
	
	uint16_t writeAddress =
		MEM_START +
		sizeof(ModuleMeta) + //Skip metadata area
		readingNum*sizeof(SingleSession);
		
	return writeAddress;
}

void readMem(volatile int16_t address, uint8_t* data, volatile uint8_t numBytes){

	//Serial.print(F("Reading memory at address "));
	//Serial.print(address);
	//Serial.print(F(" to address "));
	////Serial.print((uint8_t)data);//TODO
	//Serial.print(F(" of size "));
	//Serial.println(numBytes);
	
	for(uint8_t i=0;i<numBytes;i++) {
		
		uint16_t thisByteAddr = address+i;
		
		//Serial.print(F("Reading byte at address "));
		//Serial.print(thisByteAddr);
		//
		//Serial.print(F("...with MSB "));
		//Serial.print((int) (thisByteAddr>>8) );
		//Serial.print(F(" and LSB "));
		//Serial.print((int) (thisByteAddr&0xFF) );
		//Serial.print(F(" : "));
		//
		Wire.beginTransmission(0x50);
		Wire.write((int)thisByteAddr>>8); // msb
		Wire.write((int)thisByteAddr&0xFF); // lsb
		Wire.endTransmission();
		
		Wire.requestFrom(0x50,1); //Todo: can specify multiple?
		
		uint8_t readByte=0xFF;
		if (Wire.available())
			readByte = Wire.read();
		
		//Serial.print(F("Raw byte read:"));
		//Serial.println(readByte);
		
		*(data+i) = readByte;
	}
}

void writeMem(volatile int16_t address, uint8_t* data, volatile uint8_t numBytes){
	
	//Serial.print(F("Writing memory at address "));
	//Serial.print(address);
	//Serial.print(F(" from address "));
	////Serial.print((uint8_t)data); //TODO
	//Serial.print(F(" of size "));
	//Serial.println(numBytes);
	
	for(uint8_t i=0;i<numBytes;i++) {

		uint16_t thisByteAddr = address+i;
				
		//Serial.print(F("Writing byte at address "));
		//Serial.print(thisByteAddr);
		//
		//Serial.print(F("...with MSB "));
		//Serial.print((int) (thisByteAddr>>8) );
		//Serial.print(F(" and LSB "));
		//Serial.print((int) (thisByteAddr&0xFF) );
		//Serial.print(F(" : "));
		//Serial.println(*(data+i));
		
		Wire.beginTransmission(0x50);
		Wire.write((int)thisByteAddr>>8); // msb
		Wire.write((int)thisByteAddr&0xFF); // lsb
		Wire.write(*(data+i)); //go byte by byte
		Wire.endTransmission();
	
		delay(20); //Spec says 5 but that causes intermittent random reads at higher temperatures
	}
}

void initModule(uint8_t moduleId){
	
	//Get last reading
	ModuleMeta meta;
	meta.moduleId = moduleId;
	meta.numReadings = 0;
	writeMem(MEM_START, (uint8_t*)&meta, sizeof(ModuleMeta));

	#ifdef DEBUG
		Serial.print("Module initialised with id ");
		Serial.println(moduleId);
	#endif
}

void printData(){

	//Get last reading
	ModuleMeta meta;
	readMem(MEM_START, (uint8_t*)&meta, sizeof(ModuleMeta));

	Serial.print(F("Module #"));
	Serial.print(meta.moduleId);
	Serial.print(F(", Total #Readings: "));
	Serial.println(meta.numReadings);
		
	for(uint8_t i=0;i<meta.numReadings;i++){
		
		Serial.print(F("Reading #"));
		Serial.println(i);
		
		uint16_t readingAddr = getReadingAddress(i);
		SingleSession session;
		readMem(readingAddr, (uint8_t*)&session, sizeof(SingleSession));
		
		Serial.print(F("Gsm-Status: "));
		Serial.print(session.gsmInfo.networkStatus);
		Serial.print(F(", Gsm-RSSI: "));
		Serial.print(session.gsmInfo.rssi);
		Serial.print(F(", Gsm-Error Code: "));
		Serial.print(session.gsmInfo.errorCode);
		
		Serial.print(F(", Gps-Status: "));
		Serial.print(session.gpsInfo.gpsStatus);
		Serial.print(F(", Gps-Error Code: "));
		Serial.print(session.gpsInfo.errorCode);
		Serial.print(F(", Gps-Lat: "));
		Serial.print(session.gpsInfo.lat);
		Serial.print(F(", Gps-Lon: "));
		Serial.print(session.gpsInfo.lon);
		Serial.print(F(", Gps-Date: "));
		Serial.print(session.gpsInfo.date);
		Serial.print(F(", Gps-Heading: "));
		Serial.print(session.gpsInfo.heading);
		Serial.print(F(", Gps-Speed: "));
		Serial.println(session.gpsInfo.speed_kph);
	}
}

void on3MinutesElapsed(bool doWrite){

	#ifdef DEBUG
		Serial.println(F("3 minutes elapsed - logging..."));
	#endif	
	
	byte META_SZ = sizeof(ModuleMeta);
	byte SESSION_SZ = sizeof(SingleSession);
	
	//Get last reading
	ModuleMeta meta;
	readMem(MEM_START, (uint8_t*)&meta, sizeof(ModuleMeta));
	
	#ifdef DEBUG
		Serial.print(F("Module #"));
		Serial.print(meta.moduleId);
		Serial.print(F(", Current #Readings: "));
		Serial.println(meta.numReadings);
	#endif

	if (!doWrite)
		return;

	//Update the number of readings in metadata first so no matter what happens, existing data isnt overwritten
	meta.numReadings++;
	writeMem(MEM_START, (uint8_t*)&meta, sizeof(ModuleMeta));
	
	SingleSession session;
	gps.getGpsInfo(session.gpsInfo);
	gsm.getGsmInfo(session.gsmInfo);

	
	//Serial.print(F("Got GPS info, lat="));
	//Serial.print(session.gpsInfo.lat);
	//Serial.print(F(" lon="));
	//Serial.print(session.gpsInfo.lon);
	//Serial.print(F(" status="));
	//Serial.print(session.gpsInfo.gpsStatus);
	//Serial.print(F(" errCode="));
	//Serial.println(session.gpsInfo.errorCode);
	

	
	uint16_t writeAddress = getReadingAddress(meta.numReadings);
//	Serial.print(F("Calculated next address for data to be written to: "));
//	Serial.println(writeAddress);
	
	//Run 2 tests
	//gsm.setGPRSNetworkSettings
	String sm = "";//"Module ID:"+ModuleMeta.moduleId+" transmitting.";
	
	//gsm.sendViaSms(sm.c_str()); //TO: local number !
	//gsm.sendViaGprs(sm.c_str());
	
	writeMem(writeAddress, (uint8_t*)&session, SESSION_SZ);
}


volatile int _timerCounter = 0;
void loop() {

	++_timerCounter;
		
	delay(1000);
	
	if (ONLY_PRINT_DATA) {
		
		if (_timerCounter==1)
			printData();
		return; //No writes, informational only
	}
	
	if (INITIALISE_MODULE){
		
		return; //Should be initialising the module once and writing to it with amended firmware
	}
	
	#ifdef DEBUG
		Serial.println("Looping");
	#endif
	
	if (DIAGNOSTIC_TEST) {
	
		//Write and print every second
		on3MinutesElapsed(true);
		printData();
		return;
	}

	if (_timerCounter == 3*60)//Run once per startup, at after 3 mins
		on3MinutesElapsed(true);
}
