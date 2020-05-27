#ifndef __EXTENDEDTESTS_H__
#define __EXTENDEDTESTS_H__

#include <Arduino.h>
#include "DataTypes.h"
#include "RmMemManager.h"
#include "SensorManager.h"
#include "Adafruit_FONA.h"

class ExtendedTests
{
public:
	ExtendedTests();

	public:
		static void runExtendedMemTest(RmMemManager mem, SensorManager sensorMgr);
		static void runExtendedGsmTest(Adafruit_FONA fona);
};

#endif //__EXTENDEDTESTS_H__
