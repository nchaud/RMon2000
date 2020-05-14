#ifndef __DATATYPES_H__
#define __DATATYPES_H__


//Configuration
#define OUTPUT_DEBUG		//For logging to Serial
//#define OUTPUT_DEBUG_MEM	//For logging detailed EEPROM messages to Serial
#define DEBUG				//Prints output stmts - ONLY WITH LAPTOP - to print output statements whilst all below are running
#ifdef DEBUG
	//Set FONA module to be debug too
	#define ADAFRUIT_FONA_DEBUG
#endif

//Testing
#define IS_GSM_MOCK true		//Without connecting GSM shield
#define IS_GPS_MOCK true		//Without connecting GPS shield

#define INITIALISE_MODULE_ID	0     //0 implies don't initialise it - module start with id 1
#define IS_BASIC_MEM_TEST		false //Smoke test new module's EEPROM is physically present and working basics
#define IS_SIGNALS_MEM_TEST		false //Test reading signals and reading/writing to memory
#define ONLY_PRINT_DATA			false //Analysis/Review afterwards - no writes
//uint8_t TEST_WRITE_CYCLES = 0;	 //How many times to write&read data quickly for testing



//Fona Pins
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

//Digital Pins
#define PIN_SHUTDOWN 5
#define PIN_LED_BOTTOM_RED 8
#define PIN_LED_BOTTOM_GREEN 9
#define PIN_LED_TOP_RED 10
#define PIN_LED_TOP_GREEN 11

//Analog Pins
#define PIN_BATT_VOLTAGE A1
#define PIN_PV_VOLTAGE A0
#define PIN_TEMP A2
#define PIN_CURRENT A3

//Timing constants
#define HOURS_IN_DAY 24
#define MINS_IN_HOURS 60
#define HOURS_IN_WEEK 24*7
#define FIRST_CYCLE_NO 1

//Error constants
#define ERR_GPS_NO_FIX 10
#define ERR_GPS_NO_RESPONSE 11
#define ERR_GPS_BAD_FIELD 12

#define DATATYPE_SENSORDATA 1
#define DATATYPE_SENDDATA 2

enum FAILURE {
	FAILURE_NONE=0,
	FAILURE_WARN_NO_DATA_TO_SEND=1
};

/* What the system should do on this start-up */
enum SYS_BEHAVIOUR : uint8_t {
	DoNothing	 = 0,
	TakeReadings = 1 << 0,
	SendData	 = 1 << 1
};

enum SYS_STATE {
	SysState_None				=0,
	SysState_Initialising		=1<<14,
	SysState_InTest				=1<<1,
	
	SysState_GPRSToggleError	=1<<2,
	SysState_GPRSIsOn			=1<<3,
	SysState_GPRSSendingViaNet	=1<<4,
	SysState_GPRSSendViaNetError=1<<5,
	SysState_GPRSSendViaNetSuccess=1<<6,
	
	SysState_GPRSSendingViaSms	=1<<7,
	SysState_GPRSSendViaSmsError=1<<8,
	SysState_GPRSSendViaSmsSuccess=1<<9,
	
	SysState_ShuttingDown		=1<<10, //Known shut down, not abrupt
	SysState_TakingReadings		=1<<11,
	SysState_IsCharging			=1<<12,

	SysState_OneTimeInit		=1<<13 //One time initialisation of module-ID etc.
};

enum LED_SEL/*:byte*/ {
	Top,
	Bottom
};

enum LED_STATE { 
	Red_Solid   = 1,	 
	Red_Fast    = 2,	 
	Red_Slow    = 4, 
	//Red_Clear   = 8, 
	Green_Solid = 16, 
	Green_Fast  = 32, 
	Green_Slow  = 64, 
	//Green_Clear = 128,
	
	All_Clear	= 128
	
	//IsTemp		= 256
};

//struct TransmitReadingsResult{
	//boolean HasDataToSend=false;
	//uint16_t GsmResultCode=0;
	//uint16_t SmsResultCode=0;
//}

//TODO: Kill below, mixes gps and gsm data
#ifdef GPS
struct GpsData{
	boolean success=false;
	boolean is3DFix=false;
	uint8_t rawSz = 0;
	char* raw = NULL;      //Must be set before read
	char* gsmLoc = NULL;
	uint8_t gsmLocSz = 0;
	uint16_t gsmErrCode;
};
#endif

#ifdef OUTPUT_DEBUG_MEM
	#define RM_LOGMEM(x) \
	Serial.print(x);
	
	#define RM_LOGMEMLN(x) \
	Serial.println(x);
#else
	#define RM_LOGMEM(x)
	#define RM_LOGMEMLN(x)
#endif

#ifdef OUTPUT_DEBUG
	#define RM_LOG2(x,y)  \
	Serial.print (x);\
	Serial.print (":");\
	Serial.println (y);

	#define RM_LOG(x) \
	Serial.print(x);

	#define RM_LOGLONG(x,fmt) \
	uint64_t lower = x>>32;\
	Serial.print((uint32_t)lower,HEX);\
	Serial.print((uint32_t)x,HEX);

	#define RM_LOGFMT(x,fmt) \
	Serial.print(x,fmt);
	
	#define RM_LOGLN(x) \
	Serial.println(x);
	
	#define RM_LOGLNFMT(x,fmt) \
	Serial.println(x,fmt);
	
#else
	#define RM_LOG2(x,y)
	#define RM_LOG(x)
	#define RM_LOGLONG(x,fmt) 
	#define RM_LOGFMT(x,fmt)
	#define RM_LOGLN(x)
	#define RM_LOGLNFMT(x,fmt)
#endif


struct GsmInfo{


	//TODO: IS THAT IT?
	
	
	uint8_t errorCode;
	uint8_t rssi;
	uint8_t networkStatus;
};

struct GpsInfo{

	uint8_t errorCode; //0 implies no error
	int8_t gpsStatus;
	
	float lat;
	float lon;
	float speed_kph;
	float heading;
	float altitude;
	char date[15]; //Format yyyyMMddHHmmss with \0
};

struct SingleSession{
	GsmInfo gsmInfo;
	GpsInfo gpsInfo;
};

/* Stored at start of ROM. */
struct ModuleMeta{

	uint8_t  moduleId;
	
	/* No of times module has booted up incase numReadings cycles/fails */
	uint16_t bootCount;
	
	/* Where the next sensor data or daily cycle data can go */
	uint16_t nextFreeWriteAddr;
	
	/* Dedicated area where testing eeprom will write and read to */
	uint64_t eepromTestArea;
	
	uint16_t spareBuffer[8];
};

//struct ModuleData{
//
	//uint8_t moduleId = 0;
	//uint8_t version;   /* Software version */
	//uint32_t bootCount; /* No of times module has booted up */
	//
	///* May not be all readings, different segment for other info ? */
	//int16_t numReadings;
	//uint16_t freeStartAddress; /* Address where current readings are stored at - cyclic buffer */
//};


//TODO: All these to be uint16_t ?

struct SensorData{
	
	uint8_t  dataType = DATATYPE_SENSORDATA;
	uint16_t battVoltage;
	uint16_t current;
	uint16_t pVVoltage;
	uint16_t temperature;
	uint8_t  errorChar;/* TODO: uint16_32 for bitwise errs?*/ /* not unsigned else will get treated like int by string ctor */
	//bool           HasBeenSent	= false;
};


/* Stored in ROM and attempted to be sent every day along with readings */
struct DailyCycleData {
	
	//TODO: BITWISE OF ALL FAILURE CODE, INCLUDING IN FONA?
	
	uint8_t  DataType = DATATYPE_SENDDATA;
	//TODO: AttemptedSend ?
	unsigned long BootNo		  = 0;
	boolean GPRSToggleFailure	  = false;
	boolean GetBatteryFailure	  = false;
	uint8_t NoOfReadings		  = 0; /* NoOfReadings in this transmission */
	uint16_t GsmMessageLength	  = 0; /* Length of string that was attempted to send */
	uint16_t GsmFailureCode		  = 0;
	uint8_t SmsFailureCode		  = 0;
	uint8_t	BattPct				  = 0;
	uint8_t NetworkStatus		  = 0;
	uint8_t RSSI				  = 0;
	SYS_STATE SystemState	      = SysState_Initialising; //Bitwise combination of sys state
	
};

//typedef struct GsmTransmitData{
	//
	//unsigned int NoOfReadings = 0;
	//unsigned byte* Readings   = NULL;
	//unsigned int ReadingsLength = 0;
	//unsigned int ErrorCodes = 0;
//} GsmTransmitData;

//typedef struct SmsTransmitData{
//} SmsTransmitData;



#endif //__DATATYPES_H__