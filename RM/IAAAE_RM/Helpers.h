#ifndef __RM_HELPERS_H__
#define __RM_HELPERS_H__

#include <Arduino.h>
#include "DataTypes.h"

class Helpers {

public:
	static void printRSSI(FONA_GET_RSSI* rssi);
	static void printSensorData(SensorData* sd);
	static void printByteArray(uint8_t* sd, uint16_t length);

	static boolean isSignalGood(FONA_GET_RSSI* rssi);
		
	static void fillArray(uint8_t* ptr, uint16_t sz, uint8_t val);
	static int16_t freeMemory();
		
	static int16_t base64_encode(char *output, uint8_t *input, int16_t inputLen);
	static int16_t base64_decode(uint8_t *output, char *input, int16_t inputLen);
	static int16_t base64_enc_len(int16_t inputLen);
	static int16_t base64_dec_len(char* input, int16_t inputLen);
	
private:
	static int16_t freeListSize();
};

//byte writeCharArrWithPad(char* buffer, const char* value, byte padLength);
//
//byte writeCharWithPad(char* buffer, const char value, byte padLength);
//
//byte writeStrWithPad(char* buffer, String str, byte padLength);
//
//byte writeWithPad(char* buffer, unsigned int value, byte padLength);
//
//byte writeByteWithPad(char* buffer, byte value, byte padLength);

//byte writeCharArrToStrWithPad(String str, uint8_t idx, const char* value, byte padLength);
//byte writeStrToStrWithPad(String str, uint8_t idx, String value, byte padLength);
//byte writeToStrWithPad(String str, uint8_t idx, unsigned int value, byte padLength);

#endif