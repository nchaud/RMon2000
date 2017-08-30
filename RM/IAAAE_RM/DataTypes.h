#ifndef __DATATYPES_H__
#define __DATATYPES_H__

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

//Digital Pins
#define PIN_TIMER_SHUTDOWN 2
#define PIN_LED_BOTTOM_RED 8
#define PIN_LED_BOTTOM_GREEN 9
#define PIN_LED_TOP_RED 10
#define PIN_LED_TOP_GREEN 11

//Analog Pins
#define PIN_BATT_VOLTAGE A1
#define PIN_PV_VOLTAGE A0
#define PIN_TEMP A2
#define PIN_CURRENT A3

#define HOURS_IN_DAY 24
#define HOURS_IN_WEEK 24*7

enum FAILURE {
	FAILURE_NONE=0,
	FAILURE_WARN_NO_DATA_TO_SEND=1
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

enum LED_SEL:byte {
	Top, Bottom
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

#if GPS
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


#ifdef OUTPUT_DEBUG
	#define RM_LOG2(x,y)  \
	Serial.print (x);\
	Serial.print (":");\
	Serial.println (y);

	#define RM_LOG(x) \
	Serial.println(x);
#else
	#define RM_LOG2(x,y)
	#define RM_LOG(x)
#endif


//Memory positions of static metadata
#define MEMLOC_MODULE_ID 0
#define MEMLOC_VERSION	 4
#define MEMLOC_BOOTCOUNT 8 /* Long - 4 bytes */
#define MEMLOC_SENT_UPTO 12 /* The last Reading NUMBER (not idx) that was successfully sent out */

//The start and end positions in EEPROM of circular buffer reserved for Readings
//#define BAND_READING_START 100 /* Sizeof module data above + spare */
//The number of entries stored so far. 0=> no entries. -1 to get the idx of an entry.
#define MEMLOC_READING_ENTRY_COUNT 100 /* Long - 4 bytes */
#define MEMADDR_READING_DATA_START  104 /* Long - 4 bytes */
#define MEMADDR_READING_END 30L*1000 /* ROM is 32K  - but do we want to cycle back and lose that important historical data? Circular should be limited to 1K @ end?*/
//#define BAND_READING_CYCLE_START 28*1000 /* Will start cycling from here after it reaches the _END memory location above */


//TODO: All these to be uint16_t ?

typedef struct SensorData{
	
	unsigned int   BattVoltage	= 0;
	unsigned int   Current		= 0;
	unsigned int   PVVoltage	= 0;
	unsigned int   Temperature	= 0;
	//char		   ErrorChar	= 0; /* not unsigned else will get treated like int by string ctor */
	//bool           HasBeenSent	= false;
} SensorData;

/* Stored at start of ROM. */
typedef struct ModuleData{

	unsigned int ModuleId = 0;
	unsigned int Version  = 0;   /* Software version */
	unsigned long BootCount = 0; /* No of times module has booted up */
	
	/* May not be all readings, different segment for other info ? */
	unsigned int NoOfReadings = 0;
	unsigned int FreeStartAddress = 0; /* Address where current readings are stored at - cyclic buffer */
} ModuleData;

/* Stored in ROM after each attempted send once a day */
typedef struct DailyCycleData {
	//TODO: AttemptedSend ?
	unsigned long BootNo		  = 0;
	boolean GPRSToggleFailure	  = false;
	boolean GetBatteryFailure	  = false;
	uint8_t NoOfReadings	  = 0; /* NoOfReadings in this transmission */
	uint16_t GsmMessageLength = 0; /* Length of string that was attempted to send */
	uint16_t GsmFailureCode   = 0;
	uint8_t SmsFailureCode   = 0;
	uint8_t	 BattPct		  = 0;
	uint8_t NetworkStatus    = 0;
	uint8_t RSSI			  = 0;
	SYS_STATE SystemState	      = SysState_Initialising; //Bitwise combination of sys state
	
} DailyCycleData;

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