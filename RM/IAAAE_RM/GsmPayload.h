#ifndef __GSMPAYLOAD_H__
#define __GSMPAYLOAD_H__

#include <Arduino.h>
#include "DataTypes.h"
#include "Helpers.h"

class GsmPayload
{	
public:
	GsmPayload();
	
	static uint16_t getRawPayloadSize_S(uint8_t numberSReadings);
	static uint16_t getEncodedPayloadSize_S(uint8_t numberSReadings);
	static uint8_t readNumSReadings(char* payload, uint16_t length);
	
	//Raw bytes as an organised concatenation of all data
	uint16_t getRawPayloadSize();
	void createRawPayload(uint8_t* output);
	
	//When raw bytes data to be transmitted is formed into a Base64 encoded payload
	uint16_t getEncodedPayloadSize();
	void createEncodedPayload(char* payload);
		
	void setSensorData(SensorData* dataArr, uint8_t numberSReadings);
	void setGpsInfo(GpsInfo* info);
	
	//When received Base64 payload is parsed (Mainly required for testing/view-output to ensure encoding+decoding is ok)
	void readRawPayload(uint8_t* payload, SensorData* sReadingsArr);
	void readEncodedPayload(char* payload, uint16_t payloadSz, SensorData* sReadingsArr);
	SensorData* getSensorData();//SensorData* arr);
	GpsInfo* getGpsInfo(void);
	
	boolean hasGpsInfo(void);
	uint8_t getModuleId(void);
	void setModuleId(uint8_t moduleId);
	uint16_t getBootNumber(void);
	void setBootNumber(uint16_t bootNumber);
	FONA_GET_RSSI getRSSI(void);
	void setRSSI(FONA_GET_RSSI rssi);
	uint8_t getNumOfSensorReadings();
	
	//Internal structure at the beginning of eeprom
	struct PayloadHeader {
		uint8_t moduleId = 0;
		uint16_t bootNumber = 0;
		uint8_t dataArrSz= 0;
		uint8_t hasGpsInfo = 0;
		//uint8_t content_flags;
	
		FONA_GET_RSSI rssi;
	};
	
private:
	PayloadHeader _header;
	GpsInfo* _gpsInfo		= NULL;
	SensorData* _dataArr	= NULL;
};

#endif //__GSMPAYLOAD_H__
