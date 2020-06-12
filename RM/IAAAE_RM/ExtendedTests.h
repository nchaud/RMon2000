#ifndef __EXTENDEDTESTS_H__
#define __EXTENDEDTESTS_H__

#include <Arduino.h>
#include "DataTypes.h"
#include "Helpers.h"
#include "RmMemManager.h"
#include "SensorManager.h"
#include "GsmPayload.h"
#include "Adafruit_FONA.h"

class ExtendedTests
{
public:
	ExtendedTests();

	public:
		static void runExtendedMemTest(RmMemManager mem, SensorManager sensorMgr);
		static void runExtendedTypesTest();
		
		static void startExtendedGsmTest(Adafruit_FONA* fona, RmMemManager* mem);
		static void endExtendedGsmTest();
};

#endif //__EXTENDEDTESTS_H__
