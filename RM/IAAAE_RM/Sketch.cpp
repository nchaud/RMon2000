//Either none of exactly 1 of the below should be left in. Live is all commented out
#define UNIT_TESTS	  /* Running unit tests */
//#define INITIALISE_MODULE /* Initializes module with static data and vars */
bool _isSystemTest=true; /* Instead of live operation - triggered by switch */

//Independent of above
#define DEBUG	  /* Runs all code but debugging it */

/* How many millisecs module is off between cycles - based on Timer+RC circuit */
#define INTRA_CYCLE_DOWNTIME_SECS 3*60 //3*60 secs

//One-time initialization module ID
#ifdef INITIALISE_MODULE
	#define MODULE_ID 1
#endif

//Mock GSM
#ifdef DEBUG
	#define IS_GSM_MOCK 1
	#define IS_TIMING_MOCK 1
	#define OUTPUT_DEBUG 0 //Write print statements
#else //Either LIVE or SYSTEM_TEST
	#define IS_GSM_MOCK 0
	#define IS_TIMING_MOCK 0
	#define OUTPUT_DEBUG 0
#endif

//Set delay between cycles
#ifdef UNIT_TESTS
  #define LOOP_DELAY 1
#elif defined(DEBUG)
  #define LOOP_DELAY 300
#else
  #define LOOP_DELAY 3000
#endif

//Set FONA module to be debug too
#ifdef DEBUG
	#define ADAFRUIT_FONA_DEBUG
#endif

//GPS not in this revision
#undef GPS

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
#include "RmMemManager.h"

// this is a large buffer for replies - TODO: Kill
char replybuffer[255];
SYS_STATE _currSystemState = SysState_Initialising;

//C++ instances
RmMemManager mem;
GsmManager gsm(IS_GSM_MOCK);
Timing timer(IS_TIMING_MOCK, INTRA_CYCLE_DOWNTIME_SECS);

DailyCycleData _dailyCycleData;// = NULL;
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

//Beginning of Auto generated function prototypes by Atmel Studio
int get_free_memory();
boolean loopCycle();
void shutdown();
void execTransmitGps();
void execTransmitReadings(DailyCycleData& data);
void readSensorsAsync();
void resetAtCycleStart();
void resetSensorData();
float takeSampleAnalog(int pinNo);
float readVcc();
bool ensureBatteryLevel();
void triggerGpsRefreshAsync();
void initialiseModulePristine(unsigned int moduleId);
bool runAllTests();
bool loopSystemTest();
void persistSensorData();
boolean toggleGPRS(boolean onOff);
uint16_t prepareDataForGPRS(SensorData* readings, unsigned int noOfReadings,
		long moduleId, long bootCount, uint8_t networkStatus,
		uint8_t rssi, uint16_t batPct,
		char* strBuffer, unsigned int maxStrBuffer);
void prepareDataForSMS(SensorData* readings, unsigned int noOfReadings,
		char* strBuffer, unsigned int maxStrBuffer);
void setup_runCore();
uint8_t sendViaSms(char* data); /* 0 => success */
uint16_t sendViaGprs(char* data); /* 0 => success */


#ifdef GPS
void loadDataForGps(char buffer, int maxSize);
GpsData getGpsSensorData();
void onGpsComplete();
#endif


/*********************/
extern int __bss_end;
extern void *__brkval;
int get_free_memory()
{
  int free_memory;
  if((int)__brkval == 0)
    free_memory = ((int)&free_memory) - ((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) - ((int)__brkval);
  return free_memory;
}
/*********************/


boolean _isAtCycleStart;  //Will be true ONLY once at the 1st loop of every cycle
void setup() {
 
   unsigned char ResetSrc = MCUSR;   // TODO: save reset reason if not 0
  
   unsigned volatile int px=9;

   MCUSR = 0x00;  // cleared for next reset detection


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

	#ifdef DEBUG

	Serial.begin(115200); //Writes to Serial output
	Serial.print(F("Running"));

	#endif	

	//Mark all the outputs pins as such
	pinMode(PIN_TIMER_SHUTDOWN, OUTPUT);
	pinMode(PIN_LED_BOTTOM_GREEN, OUTPUT);
	pinMode(PIN_LED_BOTTOM_RED, OUTPUT);
	pinMode(PIN_LED_TOP_GREEN, OUTPUT);
	pinMode(PIN_LED_TOP_RED, OUTPUT);
	
	//Important to let loop() function know it's at start so can do initialisation
	_isAtCycleStart = true;
  
	if (!gsm.begin()) {
		
		//FONA library did not begin - store in ROM, terminate and don't consume power(TODO: Why would this ever happen?)
		shutdown();
	}
}

#ifdef GPS
GpsData gpsData;
#endif

boolean _isDailyCycle;    //Will be true ONLY once in 1 particular loop in 1 particular cycle
//boolean _isWeeklyCycle;   //Will be true ONLY once in 1 particular loop in 1 particular cycle

boolean _moduleHasShutdown;
unsigned long _currCycleNumber = 0;

void loop() {

	if (_moduleHasShutdown)
		return;

	volatile unsigned long currentMillis = timer.getMillis();

	//If board being powered up for the first time, start timer before running 1st cycle
	if (_isAtCycleStart) {
				
		//Update bootcount
		_currCycleNumber = 1 + mem.getLongFromMemory(MEMLOC_BOOTCOUNT);
		mem.setLongToMemory(MEMLOC_BOOTCOUNT, _currCycleNumber);
					
		_isDailyCycle = timer.isDailyCycle(_currCycleNumber);
		
		//Reset all data before new cycle begins
		//Strictly not necessary as it gets re-booted each time
		resetAtCycleStart();
	}

	timer.onCycleLoop();
	
	//Run the cycle !
	boolean doContinue = false;
	
	//Toggle LED flash first to show any error conditions etc. incase next loop errors out
	//and we can't call flash
	mem.flashLED();
	
	#ifdef UNIT_TESTS
		doContinue = runAllTests();
	#elif defined(INITIALISE_MODULE)
		doContinue = initialiseModulePristine(MODULE_ID);
	#else
		if (_isSystemTest)
			doContinue = loopSystemTest();
		else
			doContinue = loopCycle();
	#endif
	
	_isAtCycleStart = false;

	//If all work done, shut down
	if (!doContinue) {
		
		RM_LOG(F("END CYCLE"));
		shutdown();
	}

	//Try POST to site with large timeout
	//(2 minutes)

	delay(LOOP_DELAY);
}

void updateLEDs(){

	//if ((_currSystemState&SysState_GPRSToggleError)==1)
	//{
	//}

	//_currSystemState &= SysState_InTest;
	//mem.toggleLED(Bottom, LED_STATE.Green_Solid);
	//mem.toggleLED(Top, LED_STATE.Green_Solid);
}

void toggleSystemState(volatile long state){
	
	return;
	
	//Once a toggle for a non-initialisation state requested, we can switch off initialisation
	if (state != SysState_Initialising) {
		if (_currSystemState == SysState_Initialising &&
		    state != ~SysState_Initialising) {
				toggleSystemState(~SysState_Initialising);
			 }
	}
	
	if (state >= 0) {
												RM_LOG2("System State ON: ", (SYS_STATE)state);
		_currSystemState = (SYS_STATE) ((long)_currSystemState | state); //On
	}
	else {
												RM_LOG2("System State OFF: ", (SYS_STATE)~state);
		_currSystemState = (SYS_STATE) ((long)_currSystemState & state); //Off
	}
		
	updateLEDs();
}

boolean loopSystemTest() {
	
	bool doContinue;
	
	if (_isAtCycleStart) {
		
		toggleSystemState(SysState_InTest);
		
		if (!toggleGPRS(true))
		{
			//LED lights will show error condition so leave running to allow user to see (?)
		}
		
		doContinue=true;
	}
	else if (timer._at1Min) { //Wait 1 min to initialise GPRS module
												RM_LOG("System-Test : At 1 Min !");
		uint16_t retCode = sendViaGprs("Test From Module");
		
		RM_LOG2("GSM Result", retCode);
		//TODO: On failure, send via Sms?

		doContinue=true;
	}
	else if (!timer._has1Min30SecsElapsed) { //Wait 30 secs to send msg 
		doContinue=true;
	}
	else{
		toggleSystemState(~SysState_InTest);
		doContinue=false;
	}
	
	return doContinue;
}


//boolean _gpsFetchInProgress;
boolean _chargingInProgress;
//boolean _waitForTransmitInProgress;

//Behaviour
//A cycle means the module boots up, does work, shuts down for a 'few'
//A loop is the std arduino loop that runs (different) code every few ms
//Design:
//	Cycle runs (about every 3 minutes)
//		setup() called
//		loop() called every INTERVAL_DELAY += x ms
//			loop() keeps track of timings and @ each loop, calls loopCycle()
//				loopCycle() checks what time it is now and does work
//					Take continuous running-avg readings for 10 seconds
//					Persist after 1 second
//					If EOD, transmit readings
//					If Battery low, keep system up to charge it
//			loop() shuts down module when loopCycles says not to wait any more

//A cycle runs about every hour
boolean loopCycle() {
  
  boolean doContinueCycle = false;

  //Read sensors for 10 secs
  if (!timer._has10SecsElapsed) { //MATCH-R-TIME
                                                    RM_LOG(F("Reading sensors..."));
	  toggleSystemState(SysState_TakingReadings);
      readSensorsAsync();
	  
	  doContinueCycle |= true;
  }

  //At the 10 sec mark, save readings down once
  if (timer._at10Secs) { //MATCH-R-TIME	  
													RM_LOG(F("Persisting sensor data..."));
	  toggleSystemState(~SysState_TakingReadings);
	  persistSensorData();
  }
  

  //Once a day, send sensor data
  if (_isDailyCycle) {
                                                    RM_LOG(F("In Daily Cycle..."));
	//Initialize GPRS module so it can load whilst readings are collected
	if (_isAtCycleStart)
	{
		DailyCycleData reset;
		_dailyCycleData = reset;//new DailyCycleData();

		if (!toggleGPRS(true))
		{
			_dailyCycleData.GPRSToggleFailure = true;
			
			//Even if GPRS did not turn on, we continue because
			//1) we can persist this failure along with generally useful data including readings, network etc.
			//   which will be populated later in the send attempt
			//2) we don't need a separate code flow for doing the above in failure case
		}
	}
	
	//Wait a minute to acquire signal before sending the data
	if (timer._at1Min) {
		
		execTransmitReadings(_dailyCycleData);
	}
		
	//Give it time to flush the buffer before shutting down - TODO: necessary?
	if (timer._at1Min30Secs)
	{
		//Store a record of SystemStates on this daily transmit cycle
		_dailyCycleData.SystemState = _currSystemState; //TODO: Necessary?
		mem.appendDailyEntry(&_dailyCycleData);
		
		//Done sending - shut down GPRS
		toggleGPRS(false); //Ignore failure
	}
		
	doContinueCycle |= !timer._has1Min30SecsElapsed;
  }

  //Check battery level at start of a cycle
  if (_isAtCycleStart) {
                                                    RM_LOG2(F("FreeMemory"), get_free_memory());
      bool needsCharging = ensureBatteryLevel();
      _chargingInProgress = needsCharging;

      doContinueCycle |= needsCharging;
  }

  if (_chargingInProgress) {

	  toggleSystemState(SysState_IsCharging);
	  
      if (timer._at30SecInterval) {
          bool needsCharging = ensureBatteryLevel();

          //Charge for 15 mins max - battery may have low rate now
          bool doneCharging = !needsCharging || timer._has15MinElapsed;

          _chargingInProgress =   !doneCharging;
          doContinueCycle |=      !doneCharging;
      }
      else
          doContinueCycle |= true;
  }
  else{
	  toggleSystemState(~SysState_IsCharging);
  }
  
  return doContinueCycle;
}


//measure ttf ! and temp!
//discard analog reading too low
//what is ADC-voltage ??
//use fona rtc?

void setErrorByFlag(long memLoc, uint8_t errCode, SYS_STATE systemState = SysState_Initialising){
	
	//Sets error
	//Also updates system_state
	//TODO: Store system_state as a long somewhere and send
	
	//ALWAYS send, even if no readings !
	//Format of data at each transmission
	//BootNo-GSMFailures-SMSFailures-xFailures-Batt%-RSSI-...
	//TODO: need comprehensive error info for when switching out EEPROM
	//Store for each boot-no?
}

void resetAtCycleStart(){

	resetSensorData();	
}

/* Called once to initalise module - for first time in it's life only */
void initialiseModulePristine(unsigned int moduleId){

	volatile long val = SysState_OneTimeInit;
	volatile long val2 = ~SysState_OneTimeInit;
	toggleSystemState(SysState_OneTimeInit);
	
	mem.setLongToMemory(MEMLOC_MODULE_ID, moduleId);
	mem.setLongToMemory(MEMLOC_BOOTCOUNT, 0);
	mem.setLongToMemory(MEMLOC_READING_ENTRY_COUNT, 0);
	mem.setLongToMemory(MEMLOC_SENT_UPTO, 0);
	
	toggleSystemState(~SysState_OneTimeInit);
}

//Sends a high on digital pin to reset external CMOS timer
void shutdown() {

  toggleSystemState(SysState_ShuttingDown);
  
  delay(3000); //Wait for any pending writes etc.
  
  digitalWrite(PIN_TIMER_SHUTDOWN, HIGH);
  
  //TODO: If PNP is bust, will perpetually remain like this?
  _moduleHasShutdown = true; //Just incase shutdown takes some time
}


/********************************************************/
/********************** Transmission ********************/
/********************************************************/
bool toggleGPRS(boolean onOff){
	
	bool ret = gsm.enableGPRS(onOff);
	if (ret)
		toggleSystemState(SysState_GPRSIsOn); //Success
	else
		toggleSystemState(SysState_GPRSToggleError); //Failure
	return ret;
}

void execTransmitReadings(DailyCycleData& ret) {

	ret.BootNo = _currCycleNumber;
		
	//Load the readings to be sent into memory  
	byte requestedLoadCount = 2;
	unsigned long loadedUpTo = 0;
  	SensorData dszReadings[requestedLoadCount];
  	unsigned actLoadCount = mem.loadSensorData(dszReadings, requestedLoadCount, &loadedUpTo);

	ret.NoOfReadings = actLoadCount;
		
	unsigned long moduleId = mem.getLongFromMemory(MEMLOC_MODULE_ID);
	ret.RSSI = gsm.getRSSI();
	ret.NetworkStatus = gsm.getNetworkStatus();
	
	//Get battery pct
	uint16_t batPct=0;
	if (!gsm.getBattPercent(&batPct))
		ret.GetBatteryFailure = true;
	else
		ret.BattPct = batPct;

	//TODO: Make all data types consistent
	//Even if no ret.NoOfReadings == 0, still get signal etc data and store/send
	
	//char buffer
  	uint16_t DATA_BUFFER_LEN = 256;
  	char strBuffer[DATA_BUFFER_LEN]=""; //TODO: MAX?

  	ret.GsmMessageLength =
		prepareDataForGPRS(dszReadings, actLoadCount, moduleId, ret.BootNo,
						ret.NetworkStatus, ret.RSSI, batPct, strBuffer, DATA_BUFFER_LEN);

	//Send via GPRS - on failure, try SMS
	ret.GsmFailureCode = sendViaGprs(strBuffer);

	if (ret.GsmFailureCode > 0) {
														RM_LOG2(F("GPRS Failed-Trying SMS..."),ret.GsmFailureCode);
		memset(strBuffer, 0, DATA_BUFFER_LEN); //TODO: TEST
		const uint8_t SMS_LIMIT = 140;

		prepareDataForSMS(dszReadings, actLoadCount, strBuffer, SMS_LIMIT);
														RM_LOG2(F("Sending Sensors Cmpt"),strBuffer);
		ret.SmsFailureCode = sendViaSms(strBuffer);
		if (ret.SmsFailureCode > 0) {
														RM_LOG2(F("SMS Send Failed !!"), ret.SmsFailureCode);
			//If even SMS is failing, can't do much
		}
	}
	
	//Only update sent-to flag if actually changed
	if (ret.NoOfReadings > 0 &&
	   (ret.GsmFailureCode == 0 || ret.SmsFailureCode == 0)) {
			mem.markDataSent(loadedUpTo);
	}
}

uint8_t sendViaSms(char* data){
	
	toggleSystemState(SysState_GPRSSendingViaSms);
	
	uint8_t ret = gsm.sendViaSms(data);
	if (ret == 0)
		toggleSystemState(SysState_GPRSSendViaSmsSuccess);
	else
		toggleSystemState(SysState_GPRSSendViaSmsError);
		
	toggleSystemState(~SysState_GPRSSendingViaSms);
	
	return ret;
}

uint16_t sendViaGprs(char* data){
	
	toggleSystemState(SysState_GPRSSendingViaNet);
	
	volatile uint16_t ret = gsm.sendViaGprs(data);
	if (ret == 0)
		toggleSystemState(SysState_GPRSSendViaNetSuccess);
	else
		toggleSystemState(SysState_GPRSSendViaNetError);
	
	toggleSystemState(~SysState_GPRSSendingViaNet);

	return ret;
}

/********************************************************/
/********************** Sensors *************************/
/********************************************************/

uint8_t noBattReadings = 0,noPVReadings = 0,nocurrReadings = 0,noTempReadings = 0;
SensorData* currSensorData = NULL;

void resetSensorData(){
	
	//Done at start of each cycle but strictly not necessary as
	//system would have re-booted after each cycle but still here as 
	//1) we could change logic and keep it on between certain cycles - e.g. to charge batt
	//2) for unit tests
	noBattReadings=0;
	noPVReadings=0;
	nocurrReadings=0;
	noTempReadings=0;
	
	SensorData newSensorEntry;
	currSensorData = &(newSensorEntry);
}

void persistSensorData() {
	
	mem.appendSensorEntry(currSensorData);
}

void readSensorsAsync() {
	//Collect sensor data for x minutes, calc average, remove outliers
	//TODO: Should this be RMS instead?

                                                      RM_LOG(F("Reading Sensors"));
    float pvRaw   = takeSampleAnalog(PIN_PV_VOLTAGE);
    float pvAct = (pvRaw/1024.0) * 15.70589;
    
    float battRaw = takeSampleAnalog(PIN_BATT_VOLTAGE);
    float battAct = (battRaw/1024.0) * 6.0; //6=28/4.6666
    
    float currentRaw = takeSampleAnalog(PIN_CURRENT);
    float currentAct = (battRaw/1024.0) * 1.0; //TODO: Not sure about current calibration yet !

    float tempRaw = takeSampleAnalog(PIN_TEMP);
    float tempAct = (tempRaw/1024.0);
    tempAct -= 0.5;
    tempAct = tempAct / 0.01;

                                                    //TODO: Use calibrated 
                                                    RM_LOG2(F("Battery Voltage(Raw):"), battRaw);
                                                    RM_LOG2(F("Battery Voltage(Act):"), battAct);
                                                    RM_LOG2(F("PV Voltage(Raw):"), pvRaw);
                                                    RM_LOG2(F("PV Voltage(Act):"), pvAct);
                                                    //RM_LOG2(F("Temp (Raw):"), temp);
                                                    //RM_LOG2(F("Temp (Act):"), tempAct);


    //TODO: Ideally some measure of variance in addition to avg should be got
	
    //Incremental average
    if (battRaw > 0) {
      
      long currReading = currSensorData->BattVoltage * noBattReadings;
      noBattReadings++;
      currSensorData->BattVoltage = (currReading + battRaw)/noBattReadings; //Will be >0 denom because of above
    }
    
    if (pvRaw > 0) {
      
      long currReading = currSensorData->PVVoltage * noPVReadings;
      noPVReadings++;
      currSensorData->PVVoltage = (currReading + pvRaw)/noPVReadings; //Will be >0 denom because of above
    }
    
    if (currentRaw > 0) {
      
      long currReading = currSensorData->Current * nocurrReadings;
      nocurrReadings++;
      currSensorData->Current = (currReading + currentRaw)/nocurrReadings; //Will be >0 denom because of above
    }
    
    if (tempRaw > 0) {
      
      long currReading = currSensorData->Temperature * noTempReadings;
      noTempReadings++;
      currSensorData->Temperature = (currReading + tempRaw)/noTempReadings; //Will be >0 denom because of above
    }
                                                     // RM_LOG2(F("Sensors-Curr"), cData.Current);

	//TODO: If any above are 0, record it as an error ("no of 0s")
//
//#ifdef UNIT_TESTS
    //currSensorData->BattVoltage=currCycleNo;
    //currSensorData->PVVoltage=currCycleNo+1;
    //currSensorData->Current=currCycleNo+2;
    //currSensorData->Temperature=currCycleNo+3;
//#endif

}


//Returns (analog_reading * vcc)
float takeSampleAnalog(int pinNo) {
  
    int batt = analogRead(pinNo); 
    float vcc = readVcc();
    batt *= vcc;
    return batt;
}

float readVcc() {
  long result;
  // Read 1.1V reference against AVcc - TODO: does this even work ?!
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result; // Back-calculate AVcc in mV
  return result / 1000;
}

void prepareDataForSMS(SensorData* readings, unsigned int noOfReadings, 
					   char* strBuffer, unsigned int maxStrBuffer) {
	//TODO						
}

//Prepares data in format:
//ModuleID-BootCount-NetworkStatus-RSSI-BattPct-(BattCurrentPVTempErrchar,)*
uint16_t prepareDataForGPRS(SensorData* readings, unsigned int noOfReadings, 
						long moduleId, long bootCount, uint8_t networkStatus,
						uint8_t rssi, uint16_t batPct,
						char* strBuffer, unsigned int maxStrBuffer) //TODO: Take max buffer into account
{
	volatile byte offset = 0;
	char* origiBufferStart = strBuffer;
	
	//strBuffer = utoa(0, strBuffer, 10);
	offset = writeWithPad(strBuffer, moduleId,0);
	strBuffer += offset;
	
	*strBuffer++='-'; //Add a dash
	
	offset = writeWithPad(strBuffer, bootCount, 0);
	strBuffer += offset;
	
	*strBuffer++='-'; //Add a dash

	//Network status
	offset = writeByteWithPad(strBuffer, networkStatus, 0);
	strBuffer += offset;
	
	*strBuffer++='-'; //Add a dash
		
		
	//TODO: Below won't work as GPRS not on yet...?! Maybe?
	
	
	
	//Get network signal
	offset = writeByteWithPad(strBuffer, rssi, 0);
	strBuffer += offset;

	*strBuffer++='-'; //Add a dash
	
	//Get battery pct
	offset = writeWithPad(strBuffer, batPct, 0);
	strBuffer += offset;
	
	if (noOfReadings > 0) //Could have no new readings but we still send above data
	{
		*strBuffer++='-'; //Add a dash
	
		for(unsigned int i=0;i<noOfReadings;i++)
		{
			SensorData r = readings[i];
		
			offset = writeWithPad(strBuffer, r.BattVoltage, 4);
			strBuffer += offset;

			offset = writeWithPad(strBuffer, r.Current, 4);
			strBuffer += offset;

			offset = writeWithPad(strBuffer, r.PVVoltage, 4);
			strBuffer += offset;

			offset = writeWithPad(strBuffer, r.Temperature, 4);
			strBuffer += offset;


			//Each reading can optionally have an error-char
			//if (r.ErrorChar != 0)
			//{
				//*strBuffer++='E'; //Add an indicator for error-code
				//
				//unsigned volatile int testLen = strlen(&r.ErrorChar);
				//
				//offset = writeCharWithPad(strBuffer, r.ErrorChar, 0);
				//strBuffer += offset;
			//}

			if (i < noOfReadings-1)
				*strBuffer++=','; //Add a comma between reading sets
		}
	}
	
	*strBuffer++='\0'; //Terminate string
	
	return strBuffer-origiBufferStart; //TODO: Test !!
}





/********************************************************/
/********************** Battery *************************/
/********************************************************/
bool ensureBatteryLevel() {

     uint16_t vbat;
     if (!gsm.getBattPercent(&vbat)) {
                                                    RM_LOG(F("BatteryLevel Failed"));

        return false; //Don't keep up module if can't get battery level.
    } else {
                                                    RM_LOG2(F("BatteryLevel Retrieved"), vbat);

        //Require charging if less than threshold
        return vbat <= 80;
    }
}














/****************************************************/
/********************** TESTING *********************/
/****************************************************/
#ifdef UNIT_TESTS
void assertTrue(bool val)
{
	if (!val){
		volatile int failure=10/0;
	}
}

void assertInt(unsigned int expected, unsigned int actual, char* msg = NULL)
{
	volatile unsigned int stp=expected;
	
	assertTrue(expected == actual);
}

void assert(unsigned long expected, unsigned long actual, char* msg = NULL)
{
	volatile unsigned long stp=expected;
	
	assertTrue(expected == actual);
}

void assertCharStringsIdentical(const char* expected, const char* actualRaw, int len_TODO=-1)
{
	unsigned volatile int res = strcmp(expected, actualRaw)==0;
	
	assertTrue(res);
}

void assertStringsIdentical(const String& expected, const char* actualRaw, int len_TODO=-1)
{
	String actual(actualRaw);
	unsigned volatile int res = strcmp(expected.c_str(), actualRaw)==0;
	
	assertTrue(res);
}

void assertRealStringsIdentical(const String& expected, const String& actual)
{
	return assertStringsIdentical(expected, actual.c_str());
}

void assertReadingsIdentical(SensorData expected, SensorData r1)
{
	assert(expected.BattVoltage, r1.BattVoltage, "batt");
	assert(expected.Current, r1.Current, "current");
	assert(expected.PVVoltage, r1.PVVoltage, "pv");
	assert(expected.Temperature, r1.Temperature, "temp");
	//assert(expected.ErrorChar, r1.ErrorChar, "errorCode");
}

int _mockNo = 0;
SensorData createMockReading(bool append = true, char errorChar = 0)
{
	++_mockNo;
	
	SensorData r2;
	r2.BattVoltage=_mockNo*2;
	r2.Current=_mockNo*30;
	r2.PVVoltage=_mockNo*29;
	r2.Temperature=_mockNo*37;
	//r2.ErrorChar=errorChar;
	if (append)
		mem.appendSensorEntry(&r2);
	return r2;
}

void runLoadTest()
{
	//Load some fake readings
	SensorData 
		r0=createMockReading(),
		r1=createMockReading(),
		r2=createMockReading(),
		r3=createMockReading(),
		r4=createMockReading();
		
	SensorData buffer[10];
	unsigned long loadedTo;
	
	assert(5, mem.getLongFromMemory(MEMLOC_READING_ENTRY_COUNT));
	
	//Requested for 3, sent-to=0 => return last 3
	loadedTo=0;
	memset(buffer, 0, sizeof(SensorData)*10);
	mem.setLongToMemory(MEMLOC_SENT_UPTO, 0);
	mem.loadSensorData(buffer, 3, &loadedTo);
	assert(5, loadedTo);
	assertReadingsIdentical(r2, buffer[0]);
	assertReadingsIdentical(r3, buffer[1]);
	assertReadingsIdentical(r4, buffer[2]);
	assert(0, buffer[3].BattVoltage); //Sanity check - no others populated

	//Requested for 3, sent-upto=3 => return last 2
	loadedTo=0;
	memset(buffer, 0, sizeof(SensorData)*10);
	mem.setLongToMemory(MEMLOC_SENT_UPTO, 3);
	assert(2,mem.loadSensorData(buffer, 3, &loadedTo));
	assert(5, loadedTo);
	assertReadingsIdentical(r3, buffer[0]);
	assertReadingsIdentical(r4, buffer[1]);
	assert(0, buffer[2].BattVoltage); //Sanity check - no others populated
	
	//Requested for 8, sent-to=0 => return all 4
	loadedTo=0;
	memset(buffer, 0, sizeof(SensorData)*10);
	mem.setLongToMemory(MEMLOC_SENT_UPTO, 0);
	assert(5,mem.loadSensorData(buffer, 8, &loadedTo));
	assert(5, loadedTo);
	assertReadingsIdentical(r0, buffer[0]);
	assertReadingsIdentical(r1, buffer[1]);
	assertReadingsIdentical(r2, buffer[2]);
	assertReadingsIdentical(r3, buffer[3]);
	assertReadingsIdentical(r4, buffer[4]);
	assert(0, buffer[5].BattVoltage); //Sanity check - no others populated
	
	//Requested for 5, sent-to=5 => return none
	loadedTo=0;
	memset(buffer, 0, sizeof(SensorData)*10);
	mem.setLongToMemory(MEMLOC_SENT_UPTO, 5);
	assert(0, mem.loadSensorData(buffer, 5, &loadedTo));
	assert(5, loadedTo); //Loaded becomes sent-to as no extra readings input
	assert(0, buffer[0].BattVoltage); //Sanity check - no others populated
	
	//Requested for 9, sent-to=5 => return none
	loadedTo=0;
	memset(buffer, 0, sizeof(SensorData)*10);
	mem.setLongToMemory(MEMLOC_SENT_UPTO, 5);
	assert(0,mem.loadSensorData(buffer, 9, &loadedTo));
	assert(5, loadedTo);
	assert(0, buffer[0].BattVoltage); //Sanity check - no others populated

	//NONE loaded, requested for 5, sent-to=0 => return none
	initialiseModulePristine(1); //Clear existing loads
	loadedTo=0;
	memset(buffer, 0, sizeof(SensorData)*10);
	assert(0, mem.loadSensorData(buffer, 5, &loadedTo));
	assert(0, loadedTo);
	assert(0, buffer[0].BattVoltage); //Sanity check - no others populated
}

void runPadTest()
{
	byte buffSz=16;
	char buff[buffSz];

	//Integer test
	assert(1, writeWithPad(buff, 0, 0));
	assertCharStringsIdentical("0", buff);
	memset(buff, 0, buffSz);
	
	assert(3,writeWithPad(buff, 456, 0));
	assertCharStringsIdentical("456", buff);
	memset(buff, 0, buffSz);
	
	assert(4, writeWithPad(buff, 6, 4));
	assertCharStringsIdentical("0006", buff);
	memset(buff, 0, buffSz);
	
	assert(4, writeWithPad(buff, 23, 4));
	assertCharStringsIdentical("0023", buff);
	memset(buff, 0, buffSz);
	
	assert(4, writeWithPad(buff, 785, 4));
	assertCharStringsIdentical("0785", buff);
	memset(buff, 0, buffSz);
	
	assert(4, writeWithPad(buff, 1022, 4));
	assertCharStringsIdentical("1022", buff);
	memset(buff, 0, buffSz);
	
	assert(5, writeWithPad(buff, 10100, 4));
	assertCharStringsIdentical("10100", buff);
	memset(buff, 0, buffSz);


	//byte test
	assert(1, writeByteWithPad(buff, 0, 0));
	assertCharStringsIdentical("0", buff);
	memset(buff, 0, buffSz);
	
	assert(3, writeByteWithPad(buff, 138, 2));
	assertCharStringsIdentical("138", buff);
	memset(buff, 0, buffSz);
	
	assert(4, writeByteWithPad(buff, 6, 4));
	assertCharStringsIdentical("0006", buff);
	memset(buff, 0, buffSz);
	
	//char* from string test
	assert(1, writeCharArrWithPad(buff, "T", 0));
	assertCharStringsIdentical("T", buff);
	memset(buff, 0, buffSz);
	
	assert(2, writeCharArrWithPad(buff, "T", 2));
	assertCharStringsIdentical("0T", buff);
	memset(buff, 0, buffSz);
	
	assert(5, writeCharArrWithPad(buff, "", 5));
	assertCharStringsIdentical("00000", buff);
	memset(buff, 0, buffSz);
	
	assert(13, writeCharArrWithPad(buff, "Serious Error", 2));
	assertCharStringsIdentical("Serious Error", buff);
	memset(buff, 0, buffSz);
	
	//String from string test
//	String strDest(11);
//	String test((const char*)"T");
//	assert(1, writeStrToStrWithPad(strDest,0, test, 0));
//	assertRealStringsIdentical("T", strDest);
		
	//Char* test
	char testMsg[]={'T', '\0'};
	assert(2, writeCharArrWithPad(buff, testMsg, 2));
	assertCharStringsIdentical("0T", buff);
	memset(buff, 0, buffSz);
	
	char testMsg2[]={'E','r','r','o','r',' ','S','o','m','e','\0'};
	assert(10, writeCharArrWithPad(buff, testMsg2, 2));
	assertCharStringsIdentical("Error Some", buff);
	memset(buff, 0, buffSz);
	
	assert(2, writeCharArrWithPad(buff, "T", 2));
	assertCharStringsIdentical("0T", buff);
	memset(buff, 0, buffSz);
	
	assert(13, writeCharArrWithPad(buff, "Serious Error", 2));
	assertCharStringsIdentical("Serious Error", buff);
	memset(buff, 0, buffSz);
}

void runSendTest()
{
	
	//Check numbers save and load OK in correct endian order
	unsigned long testCases[7] = {0, 1, 100, 262144, 2048, 4+32+128+1024+2048,4};
	unsigned long bufferTest[7];
	
	for(int t=0 ; t<7 ; t++)
		mem.setLongToMemory((int)(bufferTest+t), testCases[t]);
	
	for(int t=0 ; t<7 ; t++)
	{
		long test = testCases[t];
		volatile unsigned long readValue = mem.getLongFromMemory((int)(bufferTest+t));
		assert(test, readValue, "long test");
	}
	
	//Check values set at address are ok and don't overwrite each other
	mem.setLongToMemory(MEMLOC_MODULE_ID, 5);
	mem.setLongToMemory(MEMLOC_BOOTCOUNT, 343); //TODO: Test for overflow - if continues ok, leave it
	mem.setLongToMemory(MEMLOC_VERSION, 843);
	mem.setLongToMemory(MEMLOC_SENT_UPTO, 5);
	assert(5, mem.getLongFromMemory(MEMLOC_MODULE_ID), "module id test");
	assert(343, mem.getLongFromMemory(MEMLOC_BOOTCOUNT), "bootcount test");
	assert(843, mem.getLongFromMemory(MEMLOC_VERSION), "version test");

	_currCycleNumber = 343; //Normally done in loop() setup
	
	//Test a single reading save
	//Simulate 5 entries already being present
	int expNoOfReadings = 5;
	mem.setLongToMemory(MEMLOC_READING_ENTRY_COUNT, expNoOfReadings);
	
	SensorData r1;
	r1.BattVoltage=1088;
	r1.Current=433;
	r1.PVVoltage=1045;
	r1.Temperature=308;
	mem.appendSensorEntry(&r1);
	
	//Check reading and should have increased entry count by now
	SensorData dszReading[1];
	unsigned long loadedTo;
	unsigned int loadCount = mem.loadSensorData(dszReading, 1, &loadedTo);
	assert(++expNoOfReadings, mem.getLongFromMemory(MEMLOC_READING_ENTRY_COUNT), "# readings");
	assert(1, loadCount, "load count");
	assertReadingsIdentical(r1, (SensorData)dszReading[0]);

	SensorData r2;
	r2.BattVoltage=7456;
	r2.Current=1785;
	r2.PVVoltage=8943;
	r2.Temperature=1866;
	//r2.ErrorChar='T';
	mem.appendSensorEntry(&r2);
	
	
	//Check reading and should have increased entry count by now
	SensorData dszReading2[1];
	loadCount = mem.loadSensorData(dszReading2, 1, &loadedTo);
	assert(++expNoOfReadings, mem.getLongFromMemory(MEMLOC_READING_ENTRY_COUNT), "# readings");
	assert(1, loadCount, "load count");
	//assert(r2.Sent, (byte)0);//TODO
	assertReadingsIdentical(r2, (SensorData)dszReading2[0]);
	
	volatile int stop5=0;
	
	//Now get last 2 readings and ensure order preserved
	SensorData dszReadings[2];
	loadCount = mem.loadSensorData(dszReadings, 2, &loadedTo);
	assert(expNoOfReadings, mem.getLongFromMemory(MEMLOC_READING_ENTRY_COUNT), "# readings");
	assert(2, loadCount, "load count");
	assertReadingsIdentical(r1, (SensorData)dszReadings[0]); //Make sure 1 isn't clobbered !
	assertReadingsIdentical(r2, (SensorData)dszReadings[1]);
	
	//Before GPRS sending, just verify last-sent-to flag value
	assert(5, mem.getLongFromMemory(MEMLOC_SENT_UPTO));

	//Send another batch of readings
	execTransmitReadings(_dailyCycleData);
	
	//First check data sent is as expected
	String actualStr = gsm.MOCK_DATA_SENT_GPRS;
	String expectedStr = "5-343-7-21-99-1088043310450308,7456178589431866";
	assertRealStringsIdentical(expectedStr, actualStr);

volatile int fake=1;
	//Ensure sent-to flag updated
	assert(5+2, mem.getLongFromMemory(MEMLOC_SENT_UPTO));
	
	//Send again. No data but still expect to receive base-level data.
	DailyCycleData reset;
	_dailyCycleData = reset;
	gsm.MOCK_DATA_SENT_GPRS = (""); //reset mock
	execTransmitReadings(_dailyCycleData);
	assertRealStringsIdentical("5-343-7-21-99", gsm.MOCK_DATA_SENT_GPRS);
	assert(5+2, mem.getLongFromMemory(MEMLOC_SENT_UPTO)); //unchanged
	
	//Add a new reading and ONLY that reading should've been sent
	gsm.MOCK_DATA_SENT_GPRS = (""); //reset mock
	SensorData newReading = createMockReading();
	execTransmitReadings(_dailyCycleData);
	volatile int len = strlen(gsm.MOCK_DATA_SENT_GPRS);
	assertTrue(strlen(gsm.MOCK_DATA_SENT_GPRS) < 35); //Could also check for commas
	assert(5+3, mem.getLongFromMemory(MEMLOC_SENT_UPTO)); //unchanged
}
	
//TODO: Test timing flags

uint16_t _testFullCycleLoopCount=0;
boolean runFullCycleTest()
{	
	//TODO: Calculate time per cycle
	
	if(++_testFullCycleLoopCount < 10) {
	
		//Call normal looping function
		boolean doContinue = loopCycle();
		return true;
	}
		
	boolean ret = true;
	
	//Let's do a 100 days of readings
	for(int dayCtr=0;dayCtr<100; dayCtr++)
	{
		//Let 23 hours pass, should end up with ?? readings
		for(int hrCtr=0;hrCtr<23;hrCtr++)
		{
			loopCycle();
			assert(0, strlen(gsm.MOCK_DATA_SENT_GPRS));
			assert(0, strlen(gsm.MOCK_DATA_SENT_SMS));
			assertTrue(!_isDailyCycle);
		}
		
		//EOD loop - after 1 day, should've transmitted
		loopCycle();
		assertTrue(_isDailyCycle);
		assertTrue(strlen(gsm.MOCK_DATA_SENT_GPRS)>0);
		assertTrue(strlen(gsm.MOCK_DATA_SENT_SMS)==0);

		//Reset
		gsm.MOCK_DATA_SENT_GPRS = "";
		gsm.MOCK_DATA_SENT_SMS = "";
	}
	
	return ret;
}

#endif //End define UNIT_TESTS

boolean runTimerTest(){
	
	Timing testTimer(true, 3*60); //say module wakes every 3 minutes
	
	testTimer.MOCK_ADVANCE_TIME(9*1000);  //not early
	testTimer.onCycleLoop();
	assert(false, testTimer._at10Secs);
	
	testTimer.MOCK_ADVANCE_TIME(11*1000); //even if time passed
	testTimer.onCycleLoop();
	assert(true, testTimer._at10Secs);
	
	testTimer.MOCK_ADVANCE_TIME(12*1000); //only once
	testTimer.onCycleLoop();
	assert(false, testTimer._at10Secs);
	
	
	//testTimer.isDailyCycle()
	
	//volatile long millis = testTimer.getMillis();
}

bool runAllTests()
{
#ifdef UNIT_TESTS

	//Non-looping run-once tests
	if (_isAtCycleStart)
	{
		initialiseModulePristine(1);
		runTimerTest();
		
		initialiseModulePristine(1);
		runSendTest();
	
		initialiseModulePristine(1);
		runPadTest();
	
		initialiseModulePristine(1);
		runLoadTest();

		initialiseModulePristine(1);
	}
	
	
	//custom initialise timing()
	
	//Looping test
	bool doContinue = runFullCycleTest();
	

#endif
	//TODO: Test cycling, averaging etc. after reached end of band
		//reset() and fast-forward time and run the loop() to manually get above values
	//TODO: Run the loop()
	//TODO:::
		//Test system shuts off after taking readings and daily after sending
		//Test data stored/sent even if no data to send with NoOfReadings value=0
		//Tests at top level with loopCycle()
		//TODO:Every xth, do a full purge of data to send via GPRS
		//TODO:Extensive logging for later diagnostics
		//TODO: See if memory can store 480+ readings !!
		//TODO: SMS FALLBACK TEST
	
	return doContinue;
}
