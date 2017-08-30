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
	volatile uint32_t _MOCK_ADVANCED_BY = 0;
	volatile uint32_t _currCycleStartTime = 0; //Milliseconds when this cycle started

	boolean __is1MinTriggered,__is1Min30SecsTriggered, __is2MinTriggered, __is10SecsTriggered;
	uint32_t __last1SecInterval;
	uint32_t __last30SecInterval;

	
	
//functions
public:
	Timing(uint8_t isMock, uint32_t readingTime, uint32_t loopDelay, uint32_t intercycleDownTime);
	~Timing();
	
	//Milliseconds since this module was last booted up
	unsigned long getMillis();
	
	boolean isDailyCycle(unsigned long currentCycleNo);
	
	void MOCK_ADVANCE_TIME(unsigned long milliseconds);
	void onCycleLoop();
};

#endif 
