#ifndef __RM_HELPERS_H__
#define __RM_HELPERS_H__

#include <Arduino.h>
#include "DataTypes.h"

class Helpers {

public:
	static void printSensorData(SensorData* sd);
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