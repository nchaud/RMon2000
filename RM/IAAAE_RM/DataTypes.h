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

//Testing - TODO: should be derived based on below values? TODO: Make behaviour here a bitwise enum?
#define IS_GSM_MOCK					true	//Without connecting GSM shield
#define IS_GPS_MOCK					true	//Without connecting GPS shield

#define INITIALISE_MODULE_ID		0		//0 implies don't initialise it - modules start with id 1
#define IS_BASIC_MEM_TEST			false   //Smoke test new module's EEPROM is physically present and working basics

//All Extended* tests define conditional code only compiled in when set - i.e. only run on the PC
#define IS_EXTENDED_SHOW_100_BYTES	false	//Prints first 100 bytes
#define IS_EXTENDED_SHOW_MEM		false	//Prints everything on this module nicely for review - TODO: A Basic?
#define IS_EXTENDED_MEM_TEST		false	//Test reading signals and reading/writing to memory
#define IS_EXTENDED_GSM_TEST		true	//Test for gprs and sending over web
#define IS_EXTENDED_TYPES_TEST		false	//Test for RMv3 types are good, esp flags

//Fona Pins
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

//Digital Pins
#define PIN_SHUTDOWN 5						//Triggers the timer to reset to switch off SYS_PWR
#define PIN_LED_BOTTOM_RED 8
#define PIN_LED_BOTTOM_GREEN 9
#define PIN_LED_TOP_RED 10
#define PIN_LED_TOP_GREEN 11
#define PIN_FONA_PWR 12						//LDO shutdown pin to switch off FONA module

//Analog Pins
#define PIN_BATT_VOLTAGE A1
#define PIN_PV_VOLTAGE A0
#define PIN_TEMP A2
#define PIN_CURRENT A3

//Logic constants - all in seconds
#define GPRS_MAX_ENABLE_TIME		60  //Max time in seconds to try to enable GPRS
#define GPRS_ENABLE_INTERVAL		10  //Must be a factor of GPRS_MAX_ENABLE_TIME
#define GPRS_MAX_SIGNAL_WAIT_TIME	300 //Wait this long to get a fix on signal
#define GPRS_SIGNAL_CHECK_INTERVAL	10  //Must be a factory of GPRS_MAX_SIGNAL_WAIT_TIME
#define GPRS_MAX_READINGS_FOR_SEND	20  //Max number of readings to send
#define GPRS_MODULE_SEND_TIMEOUT    120 //120k ms is the module's maximum time allowed to send 

//Error constants
#define ERR_GPS_NO_FIX 10
#define ERR_GPS_NO_RESPONSE 11
#define ERR_GPS_BAD_FIELD 12

enum MEM_SLOT_TYPE : uint8_t {
	NoMem		= 0,
	SensorMem	= 1,
	SentMem		= 2
};

/* What the system should do on this start-up */
enum SYS_BEHAVIOUR : uint8_t {
	DoNothing		= 0,
	TakeReadings	= 1 << 0,
	SendData		= 1 << 1,
	ExtendedGsmTest	= 1 << 2
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

enum LED_STATE : uint8_t  { 
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

enum FONA_STATUS_INIT : uint8_t  {
	
	SUCCESS_FSI			= 1,
	WARN_ATEO_FAIL		= 2,
	ERR_SERIAL_FAIL		= 3,
	ERR_FONA_SIM_MODULE	= 4
	
	//Some can be mutually exclusive, some flag'd
};
#define IS_ERR_FSI(x) \
		(x == FONA_STATUS_INIT::ERR_SERIAL_FAIL || \
		 x == FONA_STATUS_INIT::ERR_FONA_SIM_MODULE)

enum FONA_STATUS_GPRS_INIT : uint8_t  {
	
	SUCCESS_FSGI				= 1,
	WARN_CIPSHUT_BEFORE_OPENING	= 2,
	ERR_CGATT_ATTACH			= 3,
	ERR_SAPBR_SET_CONN_TYPE		= 4,
	ERR_SAPBR_SET_APN			= 5,
	ERR_SAPBR_START_APN_TASK	= 6,
	ERR_SAPBR_SET_USER			= 7,
	ERR_SAPBR_SET_PWD			= 8,
	ERR_SAPBR_WHEN_OPENING		= 9,
	ERR_CIICR_WHEN_ON_WIRELESS	= 10,
	ERR_CIPSHUT_CLOSE_SOCKET	= 11,
	ERR_SAPBR_CLOSE_GPRS		= 12,
	ERR_CGATT_DETACH			= 13,
};
#define IS_ERR_FSGI(x) \
		(x != FONA_STATUS_GPRS_INIT::SUCCESS_FSGI && \
		 x != FONA_STATUS_GPRS_INIT::WARN_CIPSHUT_BEFORE_OPENING)



//Lowest 4 bits for success/error, next 4 bits for warnings
enum FONA_STATUS_GPRS_SEND: uint8_t {
	
	//These are all mutually exclusive
	SUCCESS_FSGS			 = 1,
	ERR_HTTP_INIT			 = 2,
	ERR_CID					 = 3,
	ERR_URL					 = 4,
	ERR_SET_DATA_PARAMS		 = 5,
	ERR_SET_DATA			 = 6,
	ERR_ACTION_EXEC_SEND	 = 7,
	ERR_ACTION_GET_HTTP_CODE = 8,
	ERR_ACTION_GETLENGTH	 = 9,
	
	//These can be combined with any of the above
	WARN_CONTENT			 = 1 << 4,
	WARN_READRESPONSE		 = 1 << 5,
	WARN_UA					 = 1 << 6,
	WARN_HTTP_CODE_VALUE	 = 1 << 7
};
#define IS_ERR_FSGS(x) \
	((x&B1111) > FONA_STATUS_GPRS_SEND::SUCCESS_FSGS) //Clear warning then check
#define FSGS_OR(x,y) ((FONA_STATUS_GPRS_SEND)(x | y))



//Lowest 2 bits for result code, next 3 bits for netstat, next bit for error
enum FONA_GET_NETREG : uint8_t {
	RESULT_CODE_0 = 0,
	RESULT_CODE_1 = 1,
	RESULT_CODE_2 = 2,
	NETSTAT_0	  = 0 << 2,
	NETSTAT_1	  = 1 << 2,
	NETSTAT_2	  = 2 << 2,
	NETSTAT_3	  = 3 << 2,
	NETSTAT_4	  = 4 << 2,
	NETSTAT_5	  = 5 << 2,
	IS_ERROR	  = 1 << 5
};
//Macros to clear other bytes - nb: cast is critical for later equality checks
#define NETREG_ONLY_RESULT_CODE(val) \
	(FONA_GET_NETREG)(val & B00000011)
#define NETREG_ONLY_NETSTAT(val) \
	(FONA_GET_NETREG)(val & B00011100)
#define NETREG_ONLY_ERROR(val) \
	(FONA_GET_NETREG)(val & B00100000)
//Macros to deshift (eg. so netstat_5 comes out as number 5) - nb: no casting as no longer enum
#define NETREG_ACTUALVAL_RESULT_CODE(val) \
	NETREG_ONLY_RESULT_CODE(val)
#define NETREG_ACTUALVAL_NETSTAT(val) \
	NETREG_ONLY_NETSTAT(val)>>2
#define NETREG_ACTUALVAL_ERROR(val) \
	NETREG_ONLY_ERROR(val)>>5

struct FONA_GET_RSSI {
	
	uint8_t rssi	= 0;
	uint8_t ber		= 0; //bit error rate
	uint8_t rssiErr	= 0;
	
	FONA_GET_NETREG netReg;
};

struct INITIALISING_STATE {

	void* fona;
	boolean isComplete					  = false;
	FONA_STATUS_INIT _fonaStatusInit;
	FONA_STATUS_GPRS_INIT _gprsStatusInit;
	FONA_GET_RSSI _rssiStatusInit;
	
	//These 2 are internal use
	uint16_t _initFonaLoopCount = 0;
	uint16_t _gprsSignalLoopCount = 0;
};

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
	
	#define RM_LOGMEM2(x,y)  \
	Serial.print (x);\
	Serial.print (":");\
	Serial.println (y);
	
	#define RM_LOGMEMLN(x) \
	Serial.println(x);
#else
	#define RM_LOGMEM(x)
	#define RM_LOGMEM2(x,y)
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

struct GpsInfo{

	uint8_t errorCode=0; //0 implies no error
	int8_t gpsStatus=0;
	
	float lat=0;
	float lon=0;
	float speed_kph=0;
	float heading=0;
	float altitude=0;
	char date[15]; //Format yyyyMMddHHmmss with \0
};

/* Stored at start of ROM. */
struct ModuleMeta{

	uint8_t  moduleId = 0;
	
	/* No of times module has booted up incase numReadings cycles/fails */
	uint16_t bootCount = 0;
	
	/* No of times module has cycled back to start of EEPROM */
	uint8_t cycleMemCount = 0;
	
	/* Where the next sensor data or daily cycle data can go */
	uint16_t nextFreeWriteAddr = 0;
	
	/* Dedicated area where testing eeprom will write and read to */
	uint64_t eepromTestArea = 0;
	
	uint8_t spareBuffer[16] = {0}; //Sets all elems to 0
};


struct SensorData {
	
	uint16_t battVoltage	= 0;
	uint16_t pVVoltage		= 0;
	uint16_t current		= 0;
	uint16_t temperature	= 0;
	uint8_t  errorChar		= 0;/* TODO: uint16_32 for bitwise errs?*/ /* not unsigned else will get treated like int by string ctor */
	//bool           HasBeenSent	= false;
	
	//NB: This datatype MUST be at end for reverse traversal that happens
	MEM_SLOT_TYPE  dataType = MEM_SLOT_TYPE::SensorMem;
};

/* Stored in ROM (for later checking) to record what happened when trying to send a day's worth of readings */
struct DailyCycleData {
	
	//TODO: We can kill this now as no batt
	int8_t	BattPct				  = 0;	//-1 indicates error fetching
	
	uint16_t BootNo				  = 0;
	uint8_t NoOfReadings		  = 0;
	
	FONA_GET_RSSI RSSI;
	FONA_STATUS_INIT InitStatus	  = (FONA_STATUS_INIT)0;
	FONA_STATUS_GPRS_INIT GPRSInitStatus = (FONA_STATUS_GPRS_INIT)0;
	FONA_STATUS_GPRS_SEND SendStatus = (FONA_STATUS_GPRS_SEND)0;
	
	uint16_t ResponseHTMLCode	  = 0; //HTML code
	uint16_t ResponseLength    	  = 0;
	uint16_t ResponseId			  = 0;
	
	//NB: This datatype MUST be at end for reverse traversal that happens
	MEM_SLOT_TYPE DataType		  = MEM_SLOT_TYPE::SentMem;
};

#endif //__DATATYPES_H__