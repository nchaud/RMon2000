#include <Arduino.h>
#include <SoftwareSerial.h>

#include "Helpers.h"
#include "Adafruit_FONA.h"
#include "GsmManager.h"

GsmManager::GsmManager(uint8_t isMock)
{
	_isMock = isMock;
}

GsmManager::~GsmManager()
{
}

bool GsmManager::begin(){

	if (_isMock)
		return true;
		
	SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
	fonaSerial = &fonaSS;
	
	fonaSerial->begin(4800);
	return fona.begin(*fonaSerial);
}

bool GsmManager::getBattPercent(uint16_t* vbat){
	
	if (_isMock)
	{
		*vbat = 99;
		return true;
	}
	
	return fona.getBattPercent(vbat);
}

/* Must return 0 ONLY on success */
uint8_t GsmManager::sendViaSms(char* data) {

	if (_isMock)
	{
		MOCK_DATA_SENT_SMS = data;
		return 1;
	}

	char sendto[21]="+447968988149";
	if (!fona.sendSMS(sendto, data)) {
		return 1;
	} else {
		return 0;
	}
}

uint8_t GsmManager::getNetworkStatus()
{
	if (_isMock)
		return 7; //magic
	else
		return fona.getNetworkStatus();
}

uint8_t GsmManager::getRSSI()
{
	if (_isMock)
		return 21; //magic
	else
		return fona.getRSSI();
}


/* GPRS should be enabled, then wait for connection, then transmit */
bool GsmManager::enableGPRS(boolean switchOn)
{
	return fona.enableGPRS(switchOn);
}

/* Can return any HTTP status code. Must return 0 ONLY  on success. */
uint16_t GsmManager::sendViaGprs(char* data)
{
	if (_isMock)
	{
		RM_LOG2(F("Mocking GPRS-Send"),data);
		MOCK_DATA_SENT_GPRS = data;
		return 0;
	}
	
	RM_LOG2(F("Sending Actual data via GPRS"),data);

	uint16_t ret = 1;
	
	// Post data to website
	uint16_t statuscode;
	int16_t length;
	char* url="http://r.mkacars.org/do.php"; //TODO: should not require https?!
		                                                      
	boolean succ =
		fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length);
		
	//Try alternative APN settings - TODO: store these elsewhere for easy testing/configuration on-site
	if (!succ) {
		fona.setGPRSNetworkSettings(F("web.mtn.ci"), F(""), F(""));
		succ = fona.HTTP_POST_start(url, F("text/plain"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length);
	}

	if (!succ)
	{
		ret = 999;
	}
	else
	{
		RM_LOG2(F("GPRS Status:"), statuscode);
			                                                      
		while (length > 0) {
			while (fona.available()) {
				char c = fona.read();
					                                                      
				loop_until_bit_is_set(UCSR0A, UDRE0); /* Wait until data register empty. */
				UDR0 = c;
					                                                      
				length--;
				if (! length) break;
			}
		}
		fona.HTTP_POST_end();
			                                                      
		//200 => Success
		ret = (statuscode == 200) ? 0 : statuscode;
	}
	                                                      
	return ret;
}
