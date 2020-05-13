#ifndef __GPSMANAGER_H__
#define __GPSMANAGER_H__

#include "DataTypes.h"
#include "Adafruit_FONA.h"

class GpsManager
{
private:
	Adafruit_FONA fona = NULL;
	uint8_t _isMock;
	int8_t gpsStatus();
	
public:
	GpsManager(uint8_t isMock);
	~GpsManager();
	boolean toggleGps(boolean onOff);
	void getGpsInfo(GpsInfo& info);

	void setFona(Adafruit_FONA& fona);
}; //GpsManager

#endif //__GPSMANAGER_H__
