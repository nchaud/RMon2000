#ifndef __EXTENDEDTESTS_H__
#define __EXTENDEDTESTS_H__

#include "DataTypes.h"
#include "RmMemManager.h"
#include "SensorManager.h"

class ExtendedTests
{
public:
	ExtendedTests();

	public:
		static void runExtendedMemTest(RmMemManager mem, SensorManager sensorMgr);
};

#endif //__EXTENDEDTESTS_H__
