#include "GsmPayload.h"

// default constructor
GsmPayload::GsmPayload()
{
	
}

//TODO: Verify this all works with uint8_t* - even if it's a single byte of 255

void GsmPayload::getPayload(char* output, uint16_t maxLength){
	
	output[0] = moduleId;
	output[1] = thisBootNumber;
	//Skip 1 byte
	//output[3] = ...
	
	for(uint8_t i=0; i<_dataArrSz; i++){
		
	}
}

void GsmPayload::setPayload(char* payload){
	
}

void GsmPayload::addSensorData(SensorData* dataArr, uint8_t arraySz){

	_dataArr = dataArr;
	_dataArrSz = arraySz;	
}

void GsmPayload::getSensorData(SensorData* arr) {
	
	
}

void GsmPayload::setGpsInfo(GpsInfo* info) {}
boolean GsmPayload::hasGpsInfo(void) { return false; }
GpsInfo* GsmPayload::getGpsInfo(void){ return NULL; }
	