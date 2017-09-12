#ifndef __TIMING_H__
#define __TIMING_H__

#include "DataTypes.h"

class Timing
{
//variables
public:
	volatile uint32_t _intercycleDownTime=0;
	volatile uint32_t _loopDelay=0;
	volatile uint32_t _readingTime=0;

	//All must be in reset() too
	boolean _at10Secs;		  //Called exactly once at 10 secs
	boolean _at1Min;          //Called exactly once at 1 min
	boolean _at1Min30Secs;    //Called exactly once at 1 min 30 sec
	boolean _at2Mins;         //Called exactly once at 1 min
	boolean _at1SecInterval;
	boolean _at30SecInterval;
	boolean _has1MinElapsed;  //Gets set and latches to true after 1 min
	boolean _has1Min30SecsElapsed;
	boolean _has5MinElapsed;
	boolean _has2MinElapsed;
	boolean _has10SecsElapsed;
	boolean _has15MinElapsed; //Gets set and latches to true after 15 mins. Will auto-reset after 3 mins but nws.


protected:
private:
	uint8_t _isMock;
	volatile uint32_t _currCycleStartTime = 0; //Milliseconds when this cycle started

	//ALL must be in reset() too
	boolean __is1MinTriggered,__is1Min30SecsTriggered, __is2MinTriggered, __is10SecsTriggered;
	uint32_t __last1SecInterval;
	uint32_t __last30SecInterval;

	#ifdef UNIT_TESTS
	volatile uint32_t _MOCK_ADVANCED_BY = 0;
	#endif
	
//functions
public:
	Timing(
		uint8_t isMock, 
		uint32_t readingTime, 
		uint32_t loopDelay, 
		uint32_t intercycleDownTime);
	~Timing();
	
	void reset();

	//Milliseconds since this module was last booted up
	uint32_t getMillis();
	
	boolean isDailyCycle(uint32_t currentCycleNo);
	
	uint32_t getCyclesInOneDay();
	
	uint32_t getTimePerCycleInMs(); //TODO: Typedefs for ms type
	
	void onCycleLoop();

	#ifdef UNIT_TESTS
		void MOCK_ADVANCE_TIME(uint32_t milliseconds);
	#endif

};

#endif 
