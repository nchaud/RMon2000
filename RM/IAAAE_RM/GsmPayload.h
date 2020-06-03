#ifndef __GSMPAYLOAD_H__
#define __GSMPAYLOAD_H__

#include <Arduino.h>
#include "DataTypes.h"
#include "Helpers.h"

class GsmPayload
{
private:
	struct PayloadHeader {
		uint8_t moduleId = 0;
		uint16_t bootNumber = 0;
		uint8_t dataArrSz= 0;
		uint8_t hasGpsInfo = 0;
		//uint8_t content_flags;
		
		FONA_GET_RSSI rssi;
	};
	
	PayloadHeader _header;
	GpsInfo* _gpsInfo		= NULL;
	SensorData* _dataArr	= NULL;
	
public:
	GsmPayload();
	
	static uint16_t getRawPayloadSize_S(uint8_t numberSensorReadings);
	static uint16_t getEncodedPayloadSize_S(uint8_t numberSensorReadings);
	
	uint16_t getRawPayloadSize();
	void createRawPayload(uint8_t* output);
	
	uint16_t getEncodedPayloadSize();
	void createEncodedPayload(char* payload);
		
	//When data to be transmitted is formed into a Base64 encoded payload
	void setSensorData(SensorData* dataArr, uint8_t numberSensorReadings);
	void setGpsInfo(GpsInfo* info);
	
	//When received Base64 payload is parsed (Mainly required for testing/output to ensure encoding+decoding is ok)
	void readRawPayload(uint8_t* payload, SensorData* inputArr);
	void readEncodedPayload(char* payload, SensorData* receivedSensorData);
	SensorData* getSensorData();//SensorData* arr);
	GpsInfo* getGpsInfo(void);
	
	boolean hasGpsInfo(void);
	uint8_t getModuleId(void);
	void setModuleId(uint8_t moduleId);
	uint16_t getBootNumber(void);
	void setBootNumber(uint16_t bootNumber);
	FONA_GET_RSSI getRSSI(void);
	void setRSSI(FONA_GET_RSSI rssi);
	
	//Helpers
	static uint8_t readNumOfSensorReadings(char* payload);
	

}; //GsmPayload

#endif //__GSMPAYLOAD_H__
