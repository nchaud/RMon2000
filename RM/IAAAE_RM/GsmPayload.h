#ifndef __GSMPAYLOAD_H__
#define __GSMPAYLOAD_H__

#include <Arduino.h>
#include "DataTypes.h"

class GsmPayload
{
public:
	uint8_t moduleId = 0;
	uint16_t thisBootNumber = 0;
	FONA_GET_RSSI rssi;
	//uint8_t content_flags;
		
	GsmPayload();
	
	void getPayload(char*, uint16_t maxLength);
	void setPayload(char* payload);
	
	void addSensorData(SensorData* dataArr, uint8_t arraySz);
	void getSensorData(SensorData* arr);
	
	void setGpsInfo(GpsInfo* info);
	boolean hasGpsInfo(void);
	GpsInfo* getGpsInfo(void);
	
protected:
private:
	uint8_t _hasGpsInfo		= 0;
	GpsInfo* _gpsInfo		= NULL;
	SensorData* _dataArr	= NULL;
	uint8_t _dataArrSz		= 0;

}; //GsmPayload

#endif //__GSMPAYLOAD_H__
