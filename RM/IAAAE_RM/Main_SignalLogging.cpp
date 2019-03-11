#define DEBUG	  /* Runs all code but debugging it */


//TODO: Remove all Volatiles on deploy

/* Amend and run just once at start */
//#define INITIALISE_MODULE 1
//#define INIT_MODULE_ID 5

#define IS_TIMING_MOCK 1
#define PRINT_DATA 1


//Mock GSM
#ifdef DEBUG
#define IS_GSM_MOCK 1
#define IS_GPS_MOCK 1
//#define IS_SENSOR_MOCK 1
//#define IS_TIMING_MOCK 1
#define OUTPUT_DEBUG 0 //Write print statements

//Set FONA module to be debug too
#define ADAFRUIT_FONA_DEBUG

#else //Either LIVE or SYSTEM_TEST
#define IS_GSM_MOCK 0
#define IS_GPS_MOCK 0
//#define IS_SENSOR_MOCK 0
//#define IS_TIMING_MOCK 0
#define OUTPUT_DEBUG 0
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

//C++ instances
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
//RmMemManager mem(IS_SENSOR_MOCK);
GpsManager gps(IS_GPS_MOCK);
GsmManager gsm(IS_GSM_MOCK);

void initModule(uint8_t moduleId);
void on3MinutesElapsed();
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

	delay(3000); //for serial monitor to connect
	
	Wire.begin();
	Serial.begin(9600); //Writes to Serial output
	Serial.println(F("Starting..."));

	gps.setFona(fona);
	gsm.setFona(fona);

//	#ifdef DEBUG

	#ifdef INITIALISE_MODULE
		initModule(INIT_MODULE_ID);
	#endif

	initSubsystems();
	
	#ifdef PRINT_DATA
		printData();
	#endif
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

uint8_t getReadingAddress(uint8_t readingNum){
	
	uint8_t writeAddress =
		sizeof(ModuleMeta) + //Skip metadata area
		readingNum*sizeof(SingleSession);
		
	return writeAddress;
}

void readMem(volatile int16_t address, uint8_t* data, volatile uint8_t numBytes){
	
	for(uint8_t i=0;i<numBytes;i++) {
		
		uint16_t thisByteAddr = address+i;
		
		Wire.beginTransmission(0x50);
		Wire.write((int)thisByteAddr>>8); // msb
		Wire.write((int)thisByteAddr&0xFF); // lsb
		Wire.endTransmission();
		
		Wire.requestFrom(0x50,1); //Todo: can specify multiple?
		
		uint8_t readByte=0xFF;
		if (Wire.available())
			readByte = Wire.read();
		
		Serial.print(F("Raw byte read:"));
		Serial.println(readByte);
		
		*(data+i) = readByte;
	}

	Serial.print(F("Read memory at address "));
	Serial.print(address);
	Serial.print(F(" to address "));
	//Serial.print((uint8_t)data);//TODO
	Serial.print(F(" of size "));
	Serial.println(numBytes);	
}

void writeMem(volatile int16_t address, uint8_t* data, volatile uint8_t numBytes){
	
	Serial.print(F("Writing memory at address "));
	Serial.print(address);
	Serial.print(F(" from address "));
	//Serial.print((uint8_t)data); //TODO
	Serial.print(F(" of size "));
	Serial.println(numBytes);
	
	for(uint8_t i=0;i<numBytes;i++) {
				
		uint16_t thisByteAddr = address+i;
				
		Serial.print(F("Writing byte "));
		Serial.println(*(data+i));
		
		Wire.beginTransmission(0x50);
		Wire.write((int)thisByteAddr>>8); // msb
		Wire.write((int)thisByteAddr&0xFF); // lsb
		Wire.write(*(data+i)); //go byte by byte
		Wire.endTransmission();
	
		delay(5);
	}
}

void initModule(uint8_t moduleId){
	
	//Get last reading
	ModuleMeta meta;
	meta.moduleId = moduleId;
	meta.numReadings = 0;
	writeMem(0, (uint8_t*)&meta, sizeof(ModuleMeta));
}

void printData(){

	//Get last reading
	ModuleMeta meta;
	readMem(0, (uint8_t*)&meta, sizeof(ModuleMeta));

	Serial.print(F("This is module #"));
	Serial.println(meta.moduleId);
	Serial.print(F("# Readings in module: "));
	Serial.println(meta.numReadings);
	
	for(uint8_t i=0;i<meta.numReadings;i++){
		
		Serial.print(F("Reading #"));
		Serial.println(i);
		
		uint8_t readingAddr = getReadingAddress(i);
		SingleSession session;
		readMem(readingAddr, (uint8_t*)&session, sizeof(SingleSession));
		
		Serial.print(F("Gsm-Status: "));
		Serial.println(session.gsmInfo.networkStatus);
		Serial.print(F("Gsm-RSSI: "));
		Serial.println(session.gsmInfo.rssi);
		Serial.print(F("Gsm-Error Code: "));
		Serial.println(session.gsmInfo.errorCode);
		
		Serial.print(F("Gps-Status: "));
		Serial.println(session.gpsInfo.gpsStatus);
		Serial.print(F("Gps-Error Code: "));
		Serial.println(session.gpsInfo.errorCode);
		Serial.print(F("Gps-Lat: "));
		Serial.println(session.gpsInfo.lat);
		Serial.print(F("Gps-Lon: "));
		Serial.println(session.gpsInfo.lon);
		Serial.print(F("Gps-Date: "));
		Serial.println(session.gpsInfo.date);
		Serial.print(F("Gps-Heading: "));
		Serial.println(session.gpsInfo.heading);
		Serial.print(F("Gps-Speed: "));
		Serial.println(session.gpsInfo.speed_kph);
	}
}

//void on3MinutesElapsed(){
	//
	//Serial.println(F("3 minutes elapsed - logging..."));
	//
	//byte META_SZ = sizeof(ModuleMeta);
	//byte SESSION_SZ = sizeof(SingleSession);
	//
	////Get last reading
	//ModuleMeta meta;
	//readMem(0, (uint8_t*)&meta, META_SZ);
	//
	//Serial.print(F("Got number of readings last time as "));
	//Serial.print(meta.numReadings);
	//Serial.print(F(" with module id "));
	//Serial.print(meta.moduleId);
	//
	//uint8_t writeAddress = getReadingAddress(meta.numReadings);
	//
	//Serial.print(F("Calculated next address for data to be written to: "));
	//Serial.println(writeAddress);
	//
	////Update the number of readings in metadata first so no matter what happens, existing data isnt overwritten
	//meta.numReadings++;
	//writeMem(0, (uint8_t*)&meta, META_SZ);
	//
	//SingleSession session;
	////gps.getGpsInfo(session.gpsInfo);
	//
	//Serial.print(F("Got GPS info, lat="));
	//Serial.print(session.gpsInfo.lat);
	//Serial.print(F(" lon="));
	//Serial.print(session.gpsInfo.lon);
	//Serial.print(F(" status="));
	//Serial.print(session.gpsInfo.gpsStatus);
	//Serial.print(F(" errCode="));
	//Serial.println(session.gpsInfo.errorCode);
	//
	////gsm.getGsmInfo(session.gsmInfo);
	//
	//Serial.print(F("Calculated next address for data to be written to: "));
	//Serial.println(writeAddress);
	//
	////Run 2 tests
	////gsm.setGPRSNetworkSettings
	//String sm = "";//"Module ID:"+ModuleMeta.moduleId+" transmitting.";
	//
	////gsm.sendViaSms(sm.c_str()); //TO: local number !
	////gsm.sendViaGprs(sm.c_str());
	//
	//writeMem(writeAddress, (uint8_t*)&session, SESSION_SZ);
//}


volatile int _timerCounter = 0;
void loop() {

	//#ifdef DEBUG
		//Serial.println("Looping");
	//#endif
	//
	//#ifdef IS_TIMING_MOCK
		//if (_timerCounter==0)
			//on3MinutesElapsed();
//
		//return; //Run the write only once
	//#endif
//
	//delay(1000);
//
	//if (++_timerCounter == 3*60)//Run once per startup, at after 3 mins
		//on3MinutesElapsed();
}




//Reset module
//{
	//ReceptionLoggerMeta meta;
	//meta.numReadingsSaved = 0;
	//eeprom_update_block((void*)&meta,(void*)0,sizeof(ReceptionLoggerMeta));
//}
//
//for(int i=0;i<5;i++)
//{
	//volatile ReceptionLoggerMeta result;
	//eeprom_read_block((void*)&result,(void*)0,sizeof(ReceptionLoggerMeta));
	//volatile uint8_t numRead = result.numReadingsSaved;
	//
	//void* nextAddrPtr = (void*)(numRead*sizeof(ReceptionLoggingItem));
	//ReceptionLoggingItem newItem;
	//newItem.networkStatus = i;
	//newItem.rssi = i*10;
	//eeprom_update_block((void*)&newItem,nextAddrPtr,sizeof(ReceptionLoggerMeta));
//
	//ReceptionLoggerMeta meta;
	//meta.numReadingsSaved += 1;
	//eeprom_update_block((void*)&meta,(void*)0,sizeof(ReceptionLoggerMeta));
//}
//
//{
	//volatile ReceptionLoggerMeta result;
	//eeprom_read_block((void*)&result,(void*)0,sizeof(ReceptionLoggerMeta));
	//volatile uint8_t numRead = result.numReadingsSaved;
	//
	//volatile ReceptionLoggingItem item;
	//eeprom_read_block((void*)&item,(void*)(sizeof(ReceptionLoggingItem)*3),sizeof(ReceptionLoggingItem));
	//volatile uint8_t rssi = item.rssi;
	//
	//volatile int stop=9;
//}
//
//struct ReceptionLoggerMeta{
	//int numReadingsSaved;
//};
//
//struct ReceptionLoggingItem{
	//uint8_t networkStatus;
	//uint8_t rssi;
//};
