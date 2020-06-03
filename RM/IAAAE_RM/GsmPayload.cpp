#include "GsmPayload.h"

// default constructor
GsmPayload::GsmPayload()
{
}

uint16_t GsmPayload::getRawPayloadSize_S(uint8_t numberSensorReadings) {
	
	uint16_t payloadBytes =
		sizeof(PayloadHeader) +
		(numberSensorReadings * sizeof(SensorData));
	
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

uint8_t GsmPayload::readNumSReadings(char* payload, uint16_t length) {
	
	uint16_t decodedLength = Helpers::base64_dec_len(payload, length);
	uint16_t sensorsLength = decodedLength - sizeof(GsmPayload::PayloadHeader);
	uint8_t numReadings = sensorsLength/sizeof(SensorData);
	return numReadings;
}

void GsmPayload::createRawPayload(uint8_t* output) {
	
	memcpy(output, &_header, sizeof(_header));
	output += sizeof(_header);
	
	memcpy(output, _dataArr, _header.dataArrSz * sizeof(SensorData));
	output += sizeof(_header.dataArrSz * sizeof(SensorData));
	
	//	uint8_t* ptrToFirst = (uint8_t*)rawOutput;
	//	RM_LOG2(F("Module ID WAS "), * ((uint8_t*)(ptrToFirst+offsetof(PayloadHeader, moduleId))));
}

//Internal format that gets encoded/decoded:
// TODO: Create special char at end if this was truncated and keep a track of truncation
//		 Try keep data[100] then charEncoding[100] on stack vs new data() then data->delete() then...
void GsmPayload::createEncodedPayload(char* encodedPayload) {
	
	uint16_t payloadSz= getRawPayloadSize();
	uint8_t bytePayload[payloadSz];
	
	createRawPayload(bytePayload);
	
	uint16_t sz = Helpers::base64_encode(encodedPayload, bytePayload, payloadSz);
}

uint8_t GsmPayload::getNumOfSensorReadings() {
	return _header.dataArrSz;
}

void GsmPayload::readRawPayload(uint8_t* input, SensorData* sReadingsArr) {
	
	memcpy(&_header, input, sizeof(_header));
	input += sizeof(_header);
	
	memcpy(sReadingsArr, input, _header.dataArrSz * sizeof(SensorData));
	_dataArr = sReadingsArr;
}

void GsmPayload::readEncodedPayload(char* payload, uint16_t payloadSz, SensorData* sReadingsArr) {
	
	uint16_t rawPayloadSz = Helpers::base64_dec_len(payload, payloadSz);
	uint8_t rawPayload[rawPayloadSz];
	
	uint16_t sz = Helpers::base64_decode(rawPayload, payload, payloadSz);
	
	readRawPayload(rawPayload, sReadingsArr);
}

void GsmPayload::setSensorData(SensorData* dataArr, uint8_t arraySz) {

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
	