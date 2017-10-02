#include <Arduino.h>

#include "Helpers.h"
#include "Adafruit_FONA.h"
#include "Timing.h"

Timing::Timing(
				uint8_t isMock, 
				volatile uint32_t readingTime, 
				volatile uint32_t loopDelay, 
				volatile uint32_t intercycleDownTime)
{
	_loopDelay = loopDelay;
	_readingTime = readingTime;
	_intercycleDownTime = intercycleDownTime;
	_isMock = isMock;
}

Timing::~Timing() {}

#ifdef UNIT_TESTS
void Timing::MOCK_ADVANCE_TIME(uint32_t milliseconds){
	_MOCK_ADVANCED_BY += milliseconds;
}
#endif

void Timing::reset(){
	
	_at10Secs =
	_at1Min =
	_at1Min30Secs =
	_at2Mins =
	_at1SecInterval =
	_at30SecInterval =
	_has1MinElapsed =
	_has1Min30SecsElapsed =
	_has5MinElapsed =
	_has2MinElapsed =
	_has10SecsElapsed =
	_has15MinElapsed =
	
	_currCycleStartTime =
	
	__is1MinTriggered = 
	__is1Min30SecsTriggered = 
	__is2MinTriggered =
	__is10SecsTriggered = 
	
	__last1SecInterval =
	__last30SecInterval = 
	
	0;
	
	#ifdef UNIT_TESTS	
	_MOCK_ADVANCED_BY = 0;
	#endif
}

//- todo: if kept up by battery? whilst sending? add a EEPROM entry when cycle finishes?
uint32_t Timing::getTimePerCycleInMs(){
	uint32_t oneCycleTime = _loopDelay+_readingTime+_intercycleDownTime;
	return oneCycleTime;
}

uint32_t Timing::getCyclesInOneDay(){
	uint32_t oneCycleTime = getTimePerCycleInMs()/1000;
	uint32_t cyclesInOneDay = (24UL*60*60)/oneCycleTime;
	return cyclesInOneDay;
}

void Timing::onCycleLoop(){
	
	volatile unsigned long currentMillis = getMillis();
	
	if (_currCycleStartTime == 0)
		_currCycleStartTime = currentMillis;
	
	//How long has this cycle been running for ?
	volatile unsigned long currCycleDuration = currentMillis - _currCycleStartTime;
	//RM_LOG2(F("CurrCycleDuration"), currCycleDuration);
		
	//Some don't latch, so reset them
	_at10Secs = false;
	_at1Min = false;
	_at1Min30Secs = false;
	_at2Mins = false;
	_at30SecInterval = false;
	_at1SecInterval = false;
	//__is10SecsTriggered = false;
	//__is1MinTriggered = false;
	//__is2MinTriggered = false;


	//10 seconds
	if (currCycleDuration >= 10*1000) {
		_has10SecsElapsed = true;
			
		//One-Time @10-Secs triggered
		if (!__is10SecsTriggered) {
				
			_at10Secs = true;
			__is10SecsTriggered = true;
		}
	}
		
	//1 minute
	if (currCycleDuration >= 1L*60*1000) {
		_has1MinElapsed = true;
			
		//One-Time @1-Min triggered
		if (!__is1MinTriggered) {
				
			_at1Min = true;
			__is1MinTriggered = true;
		}
	}
		
	//1 minute 30 secs
	if (currCycleDuration >= 3L*60*1000/2) {
		_has1Min30SecsElapsed = true;
			
		//One-Time @1-Min triggered
		if (!__is1Min30SecsTriggered) {
				
			_at1Min30Secs = true;
			__is1Min30SecsTriggered = true;
		}
	}
		
	//2 minutes
	if (currCycleDuration >= 2L*60*1000) {
		_has2MinElapsed = true;
			
		//One-Time @2-Min triggered
		if (!__is2MinTriggered) {
				
			_at2Mins = true;
			__is2MinTriggered = true;
		}
	}
		
	//5 mins
	if (currCycleDuration >= 5L*60*1000)
	_has5MinElapsed=true;
		
	//15 mins
	if (currCycleDuration >= 15L*60*1000)
	_has15MinElapsed=true;
		
	//1 sec interval
	if ( (currentMillis - __last1SecInterval) >= 1L*1000) {
		_at1SecInterval = true;
		__last1SecInterval = currentMillis;
	}
		
	//30 sec interval
	if ( (currentMillis - __last30SecInterval) >= 30L*1000) {
		_at30SecInterval = true;
		__last30SecInterval = currentMillis;
	}
		
	//RM_LOG2("_isAtCycleStart",_isAtCycleStart);
	//RM_LOG("_at1Min",_at1Min);
	//RM_LOG("_has1MinElapsed",_has1MinElapsed);
	//RM_LOG("_has5MinElapsed",_has5MinElapsed);
	//RM_LOG("_has15MinElapsed",_has15MinElapsed);
	//RM_LOG("_is30SecInterval",_is30SecInterval);
	//RM_LOG("_at1SecInterval",_is1SecInterval);
	//RM_LOG2("_gpsFetchInProgress",_gpsFetchInProgress);
	//RM_LOG2("_chargingInProgress",_chargingInProgress);

}

unsigned long Timing::getMillis()
{
	if (_isMock)
	{
		//Speed up time in DEBUG mode ! // Each second=>x minutes
		volatile unsigned long currentMillis = millis();

		#ifdef UNIT_TESTS
		currentMillis += _MOCK_ADVANCED_BY;
		#endif
		
		return currentMillis;
		//unsigned long secsFromStart = currentMillis/1000;
		//currentMillis = 1L*40*60*1000*secsFromStart;
		//return currentMillis;
	}
	else
	{
		return millis();
	}
}

boolean Timing::isStartOfHourCycle(unsigned long currCycleNumber){
	
	//If so, persist data (taking into account previous readings or don't read at all?)
	if (currCycleNumber == FIRST_CYCLE_NO)
		return true;
		
	volatile long test=currCycleNumber;
		
	uint32_t durationPerCycleInSecs = getTimePerCycleInMs()/1000;
		
	volatile uint32_t prevDurationMins = ((currCycleNumber-1)*durationPerCycleInSecs)/(60);
	volatile uint32_t currDurationMins = (currCycleNumber*durationPerCycleInSecs)/(60);
	
	if ((int)prevDurationMins/MINS_IN_HOURS < ((int)currDurationMins/MINS_IN_HOURS))
		return true;
	else
		return false;
}

boolean Timing::isDailyCycle(unsigned long currCycleNumber)
{
	volatile uint32_t durationPerCycleInSecs = getTimePerCycleInMs()/1000;

	//Calc current time since module was installed
	volatile uint32_t totalDurationHrs = (currCycleNumber*durationPerCycleInSecs)/(60*60);
	
	//Check if 1 hour is almost up by looking at totalDuration of next cycle
	volatile uint32_t nextDurationHrs = ((currCycleNumber+1)*durationPerCycleInSecs)/(60*60);
	
	//Check if it's a daily/weekly cycle by seeing if next cycle will rollover
	if ((int)totalDurationHrs/HOURS_IN_DAY < (int)nextDurationHrs/HOURS_IN_DAY)
		return true;
	else
		return false;	
}


