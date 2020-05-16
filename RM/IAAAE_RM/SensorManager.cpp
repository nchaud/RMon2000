#include <Arduino.h>
#include "DataTypes.h"
#include "Helpers.h"
#include "SensorManager.h"

SensorManager::SensorManager(boolean isMock) {
	_isMock = isMock;
}

float SensorManager::readVcc() {
	long result;
	// Read 1.1V reference against AVcc - TODO: does this even work ?!
	ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
	delay(2); // Wait for Vref to settle
	ADCSRA |= _BV(ADSC); // Convert
	while (bit_is_set(ADCSRA,ADSC));
	result = ADCL;
	result |= ADCH<<8;
	result = 1125300L / result; // Back-calculate AVcc in mV
	return result / 1000;
}

//Returns (analog_reading * vcc)
uint16_t SensorManager::takeSampleAnalog(uint8_t pinNo)	{
	
	if (_isMock)
		return 5;
	
	uint16_t batt = analogRead(pinNo);
	
	//TODO: Look into !
	
	//float vcc = readVcc();
	//batt *= vcc;
	
	return batt;
}

uint8_t __mockDataCounter;
void getMockData(SensorData* ret) {
	
	++__mockDataCounter; //After 255, will roll back to 0, fine for tests
	
	ret->battVoltage = __mockDataCounter;
	ret->pVVoltage = __mockDataCounter*10;
	ret->current = __mockDataCounter%7;
	ret->temperature = __mockDataCounter*100;
	
	//return ret;
}

void SensorManager::readData(SensorData* ret) {
	
	RM_LOGLN(F("Reading sensors..."));
	
	if (_isMock) {
		getMockData(ret);
	}
	else {
		uint16_t pvRaw   = takeSampleAnalog(PIN_PV_VOLTAGE);
		uint16_t battRaw = takeSampleAnalog(PIN_BATT_VOLTAGE);
		uint16_t currentRaw = takeSampleAnalog(PIN_CURRENT);
		uint16_t tempRaw = takeSampleAnalog(PIN_TEMP);

		ret->battVoltage = battRaw;
		ret->pVVoltage = pvRaw;
		ret->current = currentRaw;
		ret->temperature = tempRaw;
	}
	
	Helpers::printSensorData(ret);
	
	//return ret;
}




