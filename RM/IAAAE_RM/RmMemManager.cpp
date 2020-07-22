#include <Arduino.h>
#include <Wire.h>
#include "DataTypes.h"
#include "Helpers.h"
#include "RmMemManager.h"


//Memory positions of static metadata
#define MEMLOC_START 0
//#define MEMLOC_MODULE_ID 0
//#define MEMLOC_VERSION	 4
//#define MEMLOC_BOOTCOUNT 8 /* Long - 4 bytes */
//#define MEMLOC_SENT_UPTO 12 /* The last Reading NUMBER (not idx) that was successfully sent out */

//The start and end positions in EEPROM of circular buffer reserved for Readings
//#define BAND_READING_START 100 /* Sizeof module data above + spare */
//The number of entries stored so far. 0=> no entries. -1 to get the idx of an entry.
//#define MEMLOC_READING_ENTRY_COUNT 100 /* Long - 4 bytes */
//#define MEMADDR_READING_DATA_START  104 /* Long - 4 bytes */
//#define MEMADDR_READING_END 30L*1000 /* ROM is 32K  - but do we want to cycle back and lose that important historical data? Circular should be limited to 1K @ end?*/
#define MEMADDRESS_MAX 0x7FFF
//#define BAND_READING_CYCLE_START 28*1000 /* Will start cycling from here after it reaches the _END memory location above */



RmMemManager::RmMemManager(boolean isMock) {
	_isMock = isMock;
	
	if (!_isMock)
		Wire.begin();
}

void internalWrite(uint16_t address, uint8_t* data, uint8_t numBytes) {
	
	RM_LOGMEM(F("Writing memory at address "));
	RM_LOGMEM(address);
	//RM_LOGMEM(F(" from address "));
	//RM_LOGMEMnt((uint8_t)data); //TODO
	RM_LOGMEM(F(" of size "));
	RM_LOGMEMLN(numBytes);
	
	for(uint8_t i=0;i<numBytes;i++) {
	
		uint16_t thisByteAddr = address+i;
	
		RM_LOGMEM(F("Writing byte at address "));
		RM_LOGMEM(thisByteAddr);
	
		RM_LOGMEM(F("...with MSB "));
		RM_LOGMEM((int) (thisByteAddr >> 8));
		RM_LOGMEM(F(" and LSB "));
		RM_LOGMEM((int) (thisByteAddr & 0xFF));
		RM_LOGMEM(F(" : "));
		RM_LOGMEMLN(*(data+i));
	
		Wire.beginTransmission(0x50);
		Wire.write((int) (thisByteAddr >> 8)); // msb
		Wire.write((int) (thisByteAddr & 0xFF)); // lsb
		Wire.write(*(data+i)); //go byte by byte
		Wire.endTransmission();
	
		//TODO: CHECK RETURN VALUE IN END-TRANSMISSION
	
		delay(15); //Spec says 5 but that causes intermittent random reads at higher temperatures
	}
}

void internalRead(uint16_t address, uint8_t* data, uint8_t numBytes) {
	
	RM_LOGMEM(F("Reading memory at address "));
	RM_LOGMEM(address);
	//RM_LOGMEM(F(" to address "));
	//RM_LOGMEM((uint8_t)data);//TODO
	RM_LOGMEM(F(" of size "));
	RM_LOGMEMLN(numBytes);
	
	for(uint8_t i=0;i<numBytes;i++) {
		
		uint16_t thisByteAddr = address+i;
		
		RM_LOGMEM(F("Reading byte at address "));
		RM_LOGMEM(thisByteAddr);
		
		RM_LOGMEM(F("...with MSB "));
		RM_LOGMEM((int) (thisByteAddr >> 8));
		RM_LOGMEM(F(" and LSB "));
		RM_LOGMEM((int) (thisByteAddr & 0xFF));
		RM_LOGMEM(F(" : "));
		
		Wire.beginTransmission(0x50);
		Wire.write((int) (thisByteAddr >> 8)); // msb
		Wire.write((int) (thisByteAddr & 0xFF)); // lsb
		Wire.endTransmission();
		
		
		//TODO: CHECK RETURN VALUE IN END-TRANSMISSION
		
		
		Wire.requestFrom(0x50,1); //Todo: can specify multiple?
		
		uint8_t readByte=0xFF;
		if (Wire.available())
			readByte = Wire.read();
		else
			RM_LOGMEMLN("WIRE NOT AVAILABLE"); //TODO: error code
		
		RM_LOGMEM(F("Raw byte read:"));
		RM_LOGMEMLN(readByte);
		
		*(data+i) = readByte;
	}
}

uint8_t RmMemManager::getUCharFromMemory(uint16_t address) {
	
	uint8_t value;
	internalRead(address, (uint8_t*)&value, sizeof(value));
	return value;
}

uint16_t RmMemManager::getUShortFromMemory(uint16_t address) {
	
	uint16_t value;
	internalRead(address, (uint8_t*)&value, sizeof(value));
	return value;
}

uint32_t RmMemManager::getUIntFromMemory(uint16_t address) {
	
	uint32_t value;
	internalRead(address, (uint8_t*)&value, sizeof(value));
	return value;
}

uint64_t RmMemManager::getULongFromMemory(uint16_t address) {
	
	uint64_t value;
	internalRead(address, (uint8_t*)&value, sizeof(value));
	return value;
}

void RmMemManager::setUCharToMemory(uint16_t address, uint8_t value) {
	
	internalWrite(address, (uint8_t*)&value, sizeof(value));
}

void RmMemManager::setUShortToMemory(uint16_t address, uint16_t value) {
	
	internalWrite(address, (uint8_t*)&value, sizeof(value));
}

void RmMemManager::setUIntToMemory(uint16_t address, uint32_t value) {
	
	internalWrite(address, (uint8_t*)&value, sizeof(value));
}

void RmMemManager::setULongToMemory(uint16_t address, uint64_t value) {
	
	internalWrite(address, (uint8_t*)&value, sizeof(value));
}

uint16_t RmMemManager::dataStartAddress() {
	
	return MEMLOC_START + sizeof(ModuleMeta);
}

void RmMemManager::initialiseModule(uint8_t moduleId) {

	RM_LOG2(F("Init Module"), moduleId);

	ModuleMeta meta;
	meta.moduleId = moduleId;
	meta.bootCount = 0;
	meta.eepromTestArea = 0;
	meta.nextFreeWriteAddr = dataStartAddress();
	memset(meta.spareBuffer, 0, sizeof(meta.spareBuffer));
	
	internalWrite(MEMLOC_START, (uint8_t*)&meta, sizeof(ModuleMeta));
}

uint16_t RmMemManager::incrementBootCount() {
	
	//TODO: Somehow verify Wire is working?
	uint16_t addr = MEMLOC_START + offsetof(ModuleMeta, bootCount);
	uint16_t currVal = getUShortFromMemory(addr);
	++currVal;
	setUShortToMemory(addr, currVal);
	return currVal;
}

void RmMemManager::incrementCycleCount() {
	
	uint16_t addr = MEMLOC_START + offsetof(ModuleMeta, cycleMemCount);
	uint8_t currVal = getUCharFromMemory(addr);
	++currVal;
	setUCharToMemory(addr, currVal);
}

uint16_t RmMemManager::getBootCount() {
	
	uint16_t addr = MEMLOC_START + offsetof(ModuleMeta, bootCount);
	uint16_t currVal = getUShortFromMemory(addr);
	return currVal;
}

uint8_t RmMemManager::getModuleId() {
	
	uint16_t addr = MEMLOC_START + offsetof(ModuleMeta, moduleId);
	uint8_t currVal = getUCharFromMemory(addr);
	return currVal;
}

uint16_t RmMemManager::verifyBasicEepRom() {
	
	//TODO: Verify of spill over 64-bit boundary what to do
	//TODO: Roll-over verification
	//TODO: Don't have strings of messages, return code which translate in a #if NOT_ENCODED_ON_DEVICE block
	
	RM_LOG(F("Basic EEPROM Test"));
	
	uint16_t MEM_TEST_LOC = offsetof(ModuleMeta, eepromTestArea);
	
	//Repeat test multiple times to check for intermittent possible errors
	uint16_t numFailures;
	
	for(uint8_t i=0;i<30;i++) {
		
		//No intersection of values so we'll know straight away if wrong bits picked
		this->setULongToMemory(MEM_TEST_LOC, 0xA946F7D8C941F9A8);
		uint64_t val8 = this->getULongFromMemory(MEM_TEST_LOC);
		if (val8 != 0xA946F7D8C941F9A8) {
			RM_LOGLN(F("FAILED LONG"));
			++numFailures;
		}
		
		this->setUIntToMemory(MEM_TEST_LOC, 0xC911F948);
		uint32_t val4 = this->getUIntFromMemory(MEM_TEST_LOC);
		if (val4 != 0xC911F948) {
			RM_LOGLN(F("FAILED INT"));
			++numFailures;
		}
		
		this->setUShortToMemory(MEM_TEST_LOC, 0x5C3A);
		uint16_t val2 = this->getUShortFromMemory(MEM_TEST_LOC);
		if (val2 != 0x5C3A) {
			RM_LOGLN(F("FAILED SHORT"));
			++numFailures;
		}
		
		this->setUCharToMemory(MEM_TEST_LOC, 0xE1);
		uint8_t val1 = this->getUCharFromMemory(MEM_TEST_LOC);
		if (val1 != 0xE1) {
			RM_LOGLN(F("FAILED CHAR"));
			++numFailures;
		}
		
		RM_LOG(F("EEPROM Test: Long="));
		RM_LOGLONG(val8, HEX);
		RM_LOG(F(", Int="));
		RM_LOGFMT(val4, HEX);
		RM_LOG(F(", Short="));
		RM_LOGFMT(val2, HEX);
		RM_LOG(F(", Char="));
		RM_LOGLNFMT(val1, HEX);
	}
	
	return numFailures;
}

void RmMemManager::runExtendedShow100Bytes() {

#if IS_EXTENDED_SHOW_100_BYTES == true
	
	RM_LOGLN(F("100 bytes..."));
	
	uint8_t raw100[100];
	internalRead(MEMLOC_START, (uint8_t*)&raw100, sizeof(raw100));
	
	for(uint8_t i=0 ; i<sizeof(raw100)/sizeof(uint8_t) ; i++){
		RM_LOGLN(raw100[i]);
	}
	
#else
	RM_LOG(F("*** FAIL SHOW 100 ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}

void RmMemManager::runExtendedDumpOutput() {

#if IS_EXTENDED_SHOW_MEM == true

	ModuleMeta meta;
	internalRead(MEMLOC_START, (uint8_t*)&meta, sizeof(ModuleMeta));
	
	RM_LOG2(F("Module #"), meta.moduleId);
	RM_LOG2(F("# Boots"), meta.bootCount);
	RM_LOG2(F("Next Addr"), meta.nextFreeWriteAddr);
	
	//Where data starts
	uint16_t currAddress = MEMLOC_START + sizeof(ModuleMeta);
	
	RM_LOG2(F("Used Up Mem"), meta.nextFreeWriteAddr - currAddress);
	
	while(true){
		MEM_SLOT_TYPE slotType;
		
		internalRead(currAddress, (uint8_t*)&slotType, sizeof(MEM_SLOT_TYPE));
		
		if (slotType == MEM_SLOT_TYPE::SensorMem) {
			
			RM_LOGLN(F("Sensor Data:..."));
			
			SensorData sd;
			internalRead(currAddress, (uint8_t*)&sd, sizeof(SensorData));
			Helpers::printSensorData(&sd);
			
			RM_LOGLN("");
			
		} else if (slotType == MEM_SLOT_TYPE::SentMem) {
			
			RM_LOG(F("Sent Data:"));
			
			DailyCycleData sd;
			internalRead(currAddress, (uint8_t*)&sd, sizeof(DailyCycleData));
			Helpers::printDailySendData(&sd);
			
			RM_LOGLN("");
			
		} else if (slotType == MEM_SLOT_TYPE::NoMem) {
			RM_LOG(F("_END_DATA_"));
		}
		
		break;
	}
#else
	RM_LOG(F("*** FAIL PRINT ***")); //Sync Broken - inclusion of code should be sync'd with flag
#endif
}
 
/* Returns the number of readings read */
uint8_t RmMemManager::loadSensorData(SensorData* buffer, uint8_t maxNoOfReadings) {

	if (mockSensorData != NULL) {
		
		memcpy(buffer, mockSensorData, sizeof(SensorData)*numMockSensorData);
		return numMockSensorData;
	}

	uint8_t numReadings = 0;
	uint16_t freeMemAddress = MEMLOC_START + offsetof(ModuleMeta, nextFreeWriteAddr);
	uint16_t currSlotStartAddr = getUShortFromMemory(freeMemAddress);
	
	while(true) {
		
		if (numReadings == maxNoOfReadings)
			break;
			
		uint16_t prevMemSlotTypeAddr = currSlotStartAddr-1; //The MEM_SLOT_TYPE address of the previous slot
	
		if (prevMemSlotTypeAddr <= (sizeof(MEMLOC_START) + sizeof(ModuleMeta)))
			break; //We're at the very beginning - either module has just been deployed or mem has wrapped (& we'll ignore this one day's readings)
	
		uint8_t slotType = (MEM_SLOT_TYPE)getUCharFromMemory(prevMemSlotTypeAddr);
		
		if (slotType == MEM_SLOT_TYPE::SensorMem) {
		
			uint16_t prevSlotStartAddr = currSlotStartAddr - sizeof(SensorData);
			internalRead(prevSlotStartAddr, (uint8_t*)buffer, sizeof(SensorData));
			
			buffer += sizeof(SensorData);
			currSlotStartAddr -= sizeof(SensorData);
			++numReadings;
		}
		else {
			
			//Reached the last SentMem slot, which has sent readings before it so we don't need to now
			break;
		}
	}

	return numReadings;
}

uint16_t RmMemManager::cycleAddressIfRequired(uint16_t currFreeAddr, uint8_t dataSz) {
	
	//If past the EEPROM, reset to start
	if (currFreeAddr + dataSz >= MEMADDRESS_MAX) {
		
		currFreeAddr = dataStartAddress();
		incrementCycleCount();
	}
	
	return currFreeAddr;
}


void RmMemManager::appendDailyEntry(DailyCycleData* r) {
	
	uint16_t freeMemAddress = MEMLOC_START + offsetof(ModuleMeta, nextFreeWriteAddr);
	uint16_t currFreeAddr = getUShortFromMemory(freeMemAddress);
	
	currFreeAddr = cycleAddressIfRequired(currFreeAddr, sizeof(DailyCycleData));
	
	RM_LOGMEM2(F("Writing Send Data To Addr"), currFreeAddr);
	internalWrite(currFreeAddr, (uint8_t*)r, sizeof(DailyCycleData));
	
	uint16_t nextFreeAddr = currFreeAddr + sizeof(DailyCycleData);
	RM_LOGMEM2(F("Free Addr Now Is"), nextFreeAddr);
	
	setUShortToMemory(freeMemAddress, nextFreeAddr);
}


void RmMemManager::appendSensorEntry(SensorData* r) {
	
	uint16_t freeMemAddress = MEMLOC_START + offsetof(ModuleMeta, nextFreeWriteAddr);
	uint16_t currFreeAddr = getUShortFromMemory(freeMemAddress);
	
	currFreeAddr = cycleAddressIfRequired(currFreeAddr, sizeof(SensorData));
	
	RM_LOGMEM2(F("Writing Sensor Data To Addr"), currFreeAddr);
	internalWrite(currFreeAddr, (uint8_t*)r, sizeof(SensorData));
	
	uint16_t nextFreeAddr = currFreeAddr + sizeof(SensorData);
	RM_LOGMEM2(F("Free Addr Now Is"), nextFreeAddr);
	
	setUShortToMemory(freeMemAddress, nextFreeAddr);
}






/* LED mgmt - Not strictly memory related */

void RmMemManager::reset() {
	
	_flashCallCount=0;
	toggleLED(Bottom, All_Clear);
	toggleLED(Top, All_Clear);
}

/* Flashes for a single LED */
void internalFlash(
	boolean& greenPinVal, boolean& redPinVal,
	LED_STATE currLedState, boolean atSlowInterval
	) {

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
void RmMemManager::flashLED() {

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
void RmMemManager::toggleLED(LED_SEL led_num, LED_STATE state) {

	if (led_num == Bottom)
		_ledBottomState = state; //(?) / Clearing? / (state&=IsTemporary)==1?
	else if (led_num == Top)
		_ledTopState = state;

	//TODO: prioritise instead?
	//TODO: Could have a hierarchy where if solid is cleared, maybe slow-flashing
	//still required?
}
