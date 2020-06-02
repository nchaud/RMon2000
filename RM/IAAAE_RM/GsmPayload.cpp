#include "GsmPayload.h"

// default constructor
GsmPayload::GsmPayload()
{
}

//Internal format that gets encoded/decoded:
// TODO: Create special char at end if this was truncated and keep a track of truncation
//		 Try keep data[100] then charEncoding[100] on stack vs new data() then data->delete() then...
void GsmPayload::createEncodedPayload(char* output, uint16_t maxLength){
	
	createPayload((uint8_t*)output, maxLength);
	
	//TODO: Encode
}

void GsmPayload::createPayload(uint8_t* output, uint16_t maxLength){
	
	memcpy(output, &moduleId, sizeof(moduleId));
	output += sizeof(moduleId);
	
	memcpy(output, &thisBootNumber, sizeof(thisBootNumber));
	output += sizeof(thisBootNumber);
	
	memcpy(output, &_dataArrSz, sizeof(_dataArrSz));
	output += sizeof(_dataArrSz);
	
	memcpy(output, _dataArr, _dataArrSz * sizeof(SensorData));
	output += sizeof(_dataArrSz * sizeof(SensorData));
	

RM_LOG2(F("Batt WAS "), * ((uint16_t*)(_dataArr+offsetof(SensorData, battVoltage))));
RM_LOG2(F("Current WAS "), * ((uint16_t*)(_dataArr+offsetof(SensorData, current))));
RM_LOG2(F("PV WAS "), * ((uint16_t*)(_dataArr+offsetof(SensorData, pVVoltage))));
RM_LOG2(F("Temperature WAS "), * ((uint16_t*)(_dataArr+offsetof(SensorData, temperature))));
	
	
}

void GsmPayload::readPayload(uint8_t* input){
	
	memcpy(&moduleId, input, sizeof(moduleId));
	input += sizeof(moduleId);
	RM_LOG2(F("Module ID was "), moduleId);
	
	memcpy(&thisBootNumber, input, sizeof(thisBootNumber));
	input += sizeof(thisBootNumber);
	RM_LOG2(F("Boot # was "), thisBootNumber);
	
	memcpy(&_dataArrSz, input, sizeof(_dataArrSz));
	input += sizeof(_dataArrSz);
	RM_LOG2(F("Data Arr Sz was "), _dataArrSz);
	
	//SensorData tmp[_dataArrSz];
	//_dataArr = tmp;
	memcpy(_dataArr, input, _dataArrSz * sizeof(SensorData));
	input += sizeof(_dataArrSz * sizeof(SensorData));
	
	Helpers::printSensorData(_dataArr);//print the first
}

void GsmPayload::readEncodedPayload(char* payload){
	
	
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
	