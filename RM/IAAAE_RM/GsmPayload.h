#ifndef __GSMPAYLOAD_H__
#define __GSMPAYLOAD_H__

#include <Arduino.h>
#include "DataTypes.h"
#include "Helpers.h"

class GsmPayload
{
public:
	uint8_t moduleId = 0;
	uint16_t thisBootNumber = 0;
	FONA_GET_RSSI rssi;
	//uint8_t content_flags;
		
	GsmPayload();
	
	void createPayload(uint8_t* output, uint16_t maxLength);
	void readPayload(uint8_t* payload, SensorData* inputArr);
		
	//When data to be transmitted is formed into a Base64 encoded payload
	void addSensorData(SensorData* dataArr, uint8_t arraySz);
	void setGpsInfo(GpsInfo* info);
	void createEncodedPayload(char* payload, uint16_t maxLength);
	
	//When received Base64 payload is parsed (Mainly required for testing/output to ensure encoding+decoding is ok)
	void readEncodedPayload(char* payload);
	SensorData* getSensorData();//SensorData* arr);
	boolean hasGpsInfo(void);
	GpsInfo* getGpsInfo(void);
	
	//Helpers
	static uint8_t readNumOfSensorReadings(char* payload);
	
protected:
private:
	uint8_t _hasGpsInfo		= 0;
	GpsInfo* _gpsInfo		= NULL;
	SensorData* _dataArr	= NULL;
	uint8_t _dataArrSz		= 0;

}; //GsmPayload

#endif //__GSMPAYLOAD_H__
