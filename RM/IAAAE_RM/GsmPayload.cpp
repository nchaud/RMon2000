#include "GsmPayload.h"

// default constructor
GsmPayload::GsmPayload()
{
}

uint16_t GsmPayload::getRawPayloadSize_S(uint8_t numberSensorReadings) {
	
	uint16_t payloadBytes =
		sizeof(PayloadHeader) +
		numberSensorReadings * sizeof(SensorData);
	
	return payloadBytes;
}

uint16_t GsmPayload::getRawPayloadSize() {
	return getRawPayloadSize_S(_header.dataArrSz);
}

uint16_t GsmPayload::getEncodedPayloadSize_S(uint8_t numberSensorReadings) {
	
	uint16_t payloadSz = getRawPayloadSize_S(numberSensorReadings);
	uint16_t encodedPayloadSz = Helpers::base64_enc_len(payloadSz);
	
	return encodedPayloadSz;
}

uint16_t GsmPayload::getEncodedPayloadSize() {
	return getEncodedPayloadSize_S(_header.dataArrSz);
}

void GsmPayload::createRawPayload(uint8_t* output){
	
	memcpy(output, &_header, sizeof(_header));
	
	memcpy(output, _dataArr, _header.dataArrSz * sizeof(SensorData));
	output += sizeof(_header.dataArrSz * sizeof(SensorData));
	
	//	uint8_t* ptrToFirst = (uint8_t*)_dataArr;
	//	RM_LOG2(F("Current WAS "), * ((uint16_t*)(ptrToFirst+offsetof(SensorData, current))));
}

//Internal format that gets encoded/decoded:
// TODO: Create special char at end if this was truncated and keep a track of truncation
//		 Try keep data[100] then charEncoding[100] on stack vs new data() then data->delete() then...
void GsmPayload::createEncodedPayload(char* encodedPayload){
	
	uint16_t payloadSz= getRawPayloadSize();
	uint8_t bytePayload[payloadSz];
	
	createRawPayload(bytePayload);
	
	uint16_t sz = Helpers::base64_encode(encodedPayload, bytePayload, payloadSz);
}

uint8_t GsmPayload::readNumOfSensorReadings(char* payload){

	uint8_t* arrSzPos = ((uint8_t*)payload) + offsetof(PayloadHeader, dataArrSz);
	uint8_t numReadings = *arrSzPos;
	return numReadings;
}

void GsmPayload::readRawPayload(uint8_t* input, SensorData* inputArr){
	
	memcpy(&_header, input, sizeof(_header));

	//SensorData tmp[_dataArrSz];
	_dataArr = inputArr;
	memcpy(_dataArr, input, _header.dataArrSz * sizeof(SensorData));
	input += sizeof(_header.dataArrSz * sizeof(SensorData));
}

void GsmPayload::readEncodedPayload(char* payload, SensorData* receivedSensorData){
	
	
}

void GsmPayload::setSensorData(SensorData* dataArr, uint8_t arraySz){

	_dataArr = dataArr;
	_header.dataArrSz = arraySz;	
}

SensorData* GsmPayload::getSensorData() {
	
	return _dataArr;
}

boolean GsmPayload::hasGpsInfo(void) { return _header.hasGpsInfo; }
uint8_t GsmPayload::getModuleId(void) { return _header.moduleId; }
void GsmPayload::setModuleId(uint8_t moduleId) { _header.moduleId = moduleId; }
uint16_t GsmPayload::getBootNumber(void) { return _header.bootNumber; }
void GsmPayload::setBootNumber(uint16_t bootNumber) { _header.bootNumber = bootNumber; }
FONA_GET_RSSI GsmPayload::getRSSI(void) { return _header.rssi; }
void GsmPayload::setRSSI(FONA_GET_RSSI rssi) { _header.rssi = rssi; }
	
	
void GsmPayload::setGpsInfo(GpsInfo* info) {}
GpsInfo* GsmPayload::getGpsInfo(void){ return NULL; }
	