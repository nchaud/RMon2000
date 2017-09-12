#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h> //TOOD: Not needed in real, only for testing

#include "RmMemManager.h"

//Their current state - true=>high
boolean _ledBottomPinRed=false, _ledBottomPinGreen=false,
_ledTopPinRed=false, _ledTopPinGreen=false;
LED_STATE _ledBottomState = All_Clear;
LED_STATE _ledTopState = All_Clear;
uint8_t _flashCallCount=0;

RmMemManager::RmMemManager(boolean isMock){
	_isMock=isMock;
}

void RmMemManager::reset(){
	_flashCallCount=0;
	toggleLED(Bottom, All_Clear);
	toggleLED(Top, All_Clear);
}

unsigned long RmMemManager::getLongFromMemory(unsigned int address)
{
	byte b1 = EEPROM.read(address);
	byte b2 = EEPROM.read(address+1);
	byte b3 = EEPROM.read(address+2);
	byte b4 = EEPROM.read(address+3);
	
	//Interpret it exactly as it's stored in memory
	byte bArr[] = {b1,b2,b3,b4};
	
	volatile long derefValue = *(long*)(&bArr);
	
	return derefValue;	
}

unsigned int /*RmMemManager::*/getUIntFromMemory(unsigned int address)
{
	byte b1 = EEPROM.read(address);
	byte b2 = EEPROM.read(address+1);
		
	//Interpret it exactly as it's stored in memory
	byte bArr[] = {b1,b2};
		
	volatile int derefValue = *(int*)(&bArr);
		
	return derefValue;
}

void RmMemManager::setLongToMemory(unsigned int address, unsigned long value)
{
	//Interpret the bytes as they're stored in memory, little or big endian, we don't care
	volatile byte byte1 = * ((byte*)&value); //LSB
	volatile byte byte2 = * (((byte*)&value)+1); //MSB
	volatile byte byte3 = * (((byte*)&value)+2); //MSB
	volatile byte byte4 = * (((byte*)&value)+3); //MSB
	
	EEPROM.write(address,   byte1);
	EEPROM.write(address+1, byte2);
	EEPROM.write(address+2, byte3);
	EEPROM.write(address+3, byte4);
}

void /* RmMemManager::*/setUIntToMemory(unsigned int address, unsigned int value)
{
	//Interpret the bytes as they're stored in memory, little or big endian, we don't care
	volatile byte byte1 = * ((byte*)&value); //LSB
	volatile byte byte2 = * (((byte*)&value)+1); //MSB
	
	EEPROM.write(address,   byte1);
	EEPROM.write(address+1, byte2);
}

/* Returns the number of readings read */
unsigned long RmMemManager::loadSensorData(SensorData* buffer, unsigned int maxNoOfReadings,
										  unsigned long* loadedUpTo)
{
	uint8_t readingSz = sizeof(SensorData);
	
	//TODO: This only takes the last few, change to average/compress?
	
	volatile unsigned long entryCount = getLongFromMemory(MEMLOC_READING_ENTRY_COUNT);
	volatile unsigned long alreadySentTo = getLongFromMemory(MEMLOC_SENT_UPTO);
	
	//We need the last {<maxNoOfReadings}. This may mean we skip from {alreadySentTo-x} onwards.
	volatile unsigned long numOfLastReadings = min(entryCount-alreadySentTo, maxNoOfReadings); //Take last n readings
	if (numOfLastReadings == 0)
	{
		*loadedUpTo = alreadySentTo; /* Nothing more */
		return 0UL; //Nothing to send
	}
	
	//Get read idx => 2 readings if 10 entry count => @ idx 8 offset
	volatile unsigned int startReadOffset = (entryCount-numOfLastReadings) * readingSz;
	volatile unsigned int startReadAddress = MEMADDR_READING_DATA_START + startReadOffset;

	byte* rPtr = (byte*)buffer;//&r2;
	
	for(volatile int readingNo = 0; readingNo < numOfLastReadings; readingNo++)
		for(volatile int byteIdx=0;byteIdx<readingSz;byteIdx++)
		{
			unsigned int currBufferOffset = (readingNo*readingSz)+byteIdx;//curr buffer offset
			unsigned int currReadAddress = startReadAddress + currBufferOffset;
			unsigned volatile int stopme=currBufferOffset;
			unsigned volatile int stopme2=currReadAddress;
			volatile byte stopx = (byte)EEPROM.read(currReadAddress);
			*(rPtr+currBufferOffset) = EEPROM.read(currReadAddress);
		}
	
	
	
	
	*loadedUpTo = entryCount; //Gets passed to markDataSent() if sent successfully
	return numOfLastReadings;
}

void RmMemManager::markDataSent(unsigned long sentUpTo)
{
	setLongToMemory(MEMLOC_SENT_UPTO, sentUpTo);
}

void RmMemManager::appendDailyEntry(DailyCycleData* r)
{
	//TODO
}

void RmMemManager::replaceLastSensorEntry(SensorData* r)
{
	
}

void RmMemManager::appendSensorEntry(SensorData* r)
{
	volatile unsigned int readingSz = sizeof(SensorData); //const
	
	//Read where next entry is free
	volatile unsigned long entryCount = getLongFromMemory(MEMLOC_READING_ENTRY_COUNT);
	volatile unsigned long nextFreeOffset = entryCount * sizeof(SensorData);
	volatile unsigned long nextFreeAddress = MEMADDR_READING_DATA_START + nextFreeOffset;

	//TODO: modulo both free address (AND entry count?)

	byte* rPtr = (byte*)r;

	for(int i=0;i<sizeof(SensorData);i++)
		EEPROM.write(nextFreeAddress+i, *(rPtr+i));

	//Update entry count
	setLongToMemory(MEMLOC_READING_ENTRY_COUNT, entryCount+1);
	volatile int stop=2;
	
	/* Can we do this even with writing page limitations ? */
	
	//If it goes beyond the end, restart from beginning
	//uint32_t structureSize = sizeof(Reading);
	//if (nextFreeMemAddr+structureSize > BAND_READING_END)
	//	nextFreeMemAddr = BAND_READING_START;
	
	//unsigned int nextFreeAddr = writeEEPROM(nextFreeMemAddr, (byte*)&r, sizeof(Reading));
	
	//IF success, update the location at which to write the next Reading
	//writeEEPROM(MEMLOC_READING_DATA_START, (byte*)&nextFreeAddr, 2);
}


/* LED mgmt - Not strictly memory related */

/* Flashes for a single LED */
void internalFlash(
	boolean& greenPinVal, boolean& redPinVal,
	LED_STATE currLedState, boolean atSlowInterval
	)
{
	//Do green LED first
	//boolean nextState = greenPinVal;//-1=no action, 0=false, 1=true
	
	if (currLedState == Green_Fast) {
		greenPinVal = !greenPinVal;// currPinValue==-1?1:!currLedState; //toggle
	}
	else if (currLedState == Green_Slow) {
		greenPinVal = atSlowInterval;//!greenPinVal;// currPinValue==-1?1:!currPinValue; //toggle
	}
	else if (currLedState == Green_Solid) {
		greenPinVal = true;// currPinValue==1?-1:1; //Must be solid if not already
	}
	else if (currLedState == All_Clear) {
		greenPinVal = false;//currPinValue==0?-1:0; //Must be clear if not already
	}
	
	//newGreenPinVal = nextState;
	//if (nextState != -1 && nextState != greenPinVal)
	//	digitalWrite(greenPinNo, nextState);	
		
	//Do red LED next
	//nextState = redPinVal;// -1;//-1=no action, 0=false, 1=true
	
	if (currLedState == Red_Fast) {
		redPinVal = !redPinVal;// currPinValue==-1?1:!currLedState; //toggle
	}
	else if (currLedState == Red_Slow) {
		redPinVal = atSlowInterval;//!greenPinVal;// currPinValue==-1?1:!currPinValue; //toggle
	}
	else if (currLedState == Red_Solid) {
		redPinVal = true;// currPinValue==1?-1:1; //Must be solid if not already
	}
	else if (currLedState == All_Clear) {
		redPinVal = false;//currPinValue==0?-1:0; //Must be clear if not already
	}
	
	//newRedPinVal &= redPinVal;
	//if (nextState != -1 && nextState != redPinVal)
	//	digitalWrite(redPinNo, nextState);
}


//Called at regular intervals at a fast-rate to toggle LEDs between off-on
void RmMemManager::flashLED()
{
	//Every 3 flashes, do a slow blink
	_flashCallCount = ++_flashCallCount%3;

	//Flash Bottom LED	
	internalFlash(
				_ledBottomPinRed,_ledBottomPinGreen, _ledBottomState,
				_flashCallCount==0
				);
	digitalWrite(PIN_LED_BOTTOM_GREEN, _ledBottomPinGreen);
	digitalWrite(PIN_LED_BOTTOM_RED, _ledBottomPinRed);
				
	//Flash Top LED
	internalFlash(
				_ledTopPinGreen, _ledTopPinRed, _ledTopState,
				_flashCallCount==0
				);
	digitalWrite(PIN_LED_TOP_GREEN, _ledTopPinGreen);
	digitalWrite(PIN_LED_TOP_RED, _ledTopPinRed);
}

//Request to change the state of an LED
void RmMemManager::toggleLED(LED_SEL led_num, LED_STATE state)
{
	if (led_num == Bottom)
		_ledBottomState = state; //(?) / Clearing? / (state&=IsTemporary)==1?
	else if (led_num == Top)
		_ledTopState = state;

	//TODO: prioritise instead?
	//TODO: Could have a hierarchy where if solid is cleared, maybe slow-flashing
	//still required?
}

//Returns (analog_reading * vcc)
//TODO: Doesn't really belong in memory manager class :|
float RmMemManager::takeSampleAnalog(int pinNo)	{
	if (_isMock)
		return 5;
		
	int batt = analogRead(pinNo); 
	float vcc = readVcc();
	batt *= vcc;
	return batt;
}

float RmMemManager::readVcc() {
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

