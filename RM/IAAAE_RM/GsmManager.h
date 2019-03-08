#ifndef __GSMMANAGER_H__
#define __GSMMANAGER_H__

#include <SoftwareSerial.h>
#include "DataTypes.h"
#include "Adafruit_FONA.h"

class GsmManager
{
//variables
public:

	char* MOCK_DATA_SENT_GPRS; /* Last data sent via GPRS */
	char* MOCK_DATA_SENT_SMS;  /* Last data sent via SMS */

protected:
private:
	volatile uint8_t _isMock;
	Adafruit_FONA fona = NULL;
	SoftwareSerial* fonaSerial;

//functions
public:
	GsmManager(/*const Adafruit_FONA fonaInstance, */uint8_t isMock);
	~GsmManager();

	void reset();
	
	//Initializes the FONA CPP module and GSM module
	boolean begin();
	
	void setFona(Adafruit_FONA& fonaInstance);
	
	boolean enableGPRS(boolean switchOn);
	
	boolean getBattPercent(uint16_t* vbat);
		
	/* Can return any HTTP status code. Must return 0 ONLY  on success. */
	uint16_t sendViaGprs(const char* data);
	/* Must return 0 ONLY on success */
	uint8_t sendViaSms(const char* data);
	
	uint8_t getNetworkStatus();
	uint8_t getRSSI();
	
	void getGsmInfo(GsmInfo& info);
	
protected:
private:
//	GsmManager( const GsmManager &c );
//	GsmManager& operator=( const GsmManager &c );

}; //

#endif 
