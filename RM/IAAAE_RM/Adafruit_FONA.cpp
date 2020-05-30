// next line per http://postwarrior.com/arduino-ethershield-error-prog_char-does-not-name-a-type/

#include "Adafruit_FONA.h"

Adafruit_FONA::Adafruit_FONA(int8_t rst, boolean isMock)
{
  _rstpin = rst;
  _isMock = isMock;

  apn = F("FONAnet");
  apnusername = 0;
  apnpassword = 0;
  mySerial = 0;
  httpsredirect = true;
  useragent = String("IAAAE_RMonV3");
  ok_reply = F("OK");
}

uint8_t Adafruit_FONA::type(void) {
  return _type;
}

FONA_STATUS_INIT Adafruit_FONA::begin(uint8_t tx, uint8_t rx) { //Stream &port) {

  FONA_STATUS_INIT ret = FONA_STATUS_INIT::SUCCESS_FSI;
  
  mySerial = new SoftwareSerial(tx, rx);
  mySerial->begin(4800);

  pinMode(_rstpin, OUTPUT);
  digitalWrite(_rstpin, HIGH);
  delay(10);
  digitalWrite(_rstpin, LOW);
  delay(100);
  digitalWrite(_rstpin, HIGH);

  DEBUG_PRINTLN(F("Attempting to open comm with ATs"));
  // give 7 seconds to reboot
  int16_t timeout = 7000;

  while (timeout > 0) {
	  
    while (mySerial->available()) {
		mySerial->read();
	}
	
	//TODO: If serial error?
	//TODO: I believe this is to synchronise the baud rate - https://arduino.stackexchange.com/a/36042
	
    if (sendCheckReply(F("AT"), ok_reply))
      break;
	  
    while (mySerial->available())
		mySerial->read();
		
    if (sendCheckReply(F("AT"), F("AT"))) 
      break;
	  
    delay(500);
    timeout-=500;
  }

  if (timeout <= 0) {
	  
    sendCheckReply(F("AT"), ok_reply);
    delay(100);
    sendCheckReply(F("AT"), ok_reply);
    delay(100);
    
	if (!sendCheckReply(F("AT"), ok_reply)){
		
		DEBUG_PRINTLN(F("AT Fail... last ditch attempt."));
		return FONA_STATUS_INIT::ERR_SERIAL_FAIL;
	}
    delay(100);
  }

  // turn off Echo!
  sendCheckReply(F("ATE0"), ok_reply);
  delay(100);

  if (!sendCheckReply(F("ATE0"), ok_reply)){
	  
	  ret = FONA_STATUS_INIT::WARN_ATEO_FAIL;
  }
  delay(100);
  
  flushInput();
  
  //Make sure right version of FONA module is connected - SIM808 R14.18
  {
	  DEBUG_PRINT(F("\t---> "));
	  DEBUG_PRINTLN("ATI");

	  mySerial->println("ATI");
	  readline(500, true);

	  DEBUG_PRINT(F("\t<--- "));
	  DEBUG_PRINTLN(replybuffer);
  
	  if (prog_char_strstr(replybuffer, (prog_char *)F("SIM808 R14")) == 0) {
    
		return FONA_STATUS_INIT::ERR_FONA_SIM_MODULE;
	  }
  }

  return ret;
}


/********* Power off ********************************************/
boolean Adafruit_FONA::powerOff() {
	return sendCheckReply(F("AT+CPOWD="), 1, F("NORMAL POWER DOWN"));
}

boolean Adafruit_FONA::toggleCharging(boolean onOff) {
	if (onOff)
		return sendCheckReply(F("AT+ECHARGE=1"), ok_reply);
	else
		return sendCheckReply(F("AT+ECHARGE=0"), ok_reply);
}


/********* BATTERY & ADC ********************************************/

/* returns value in mV (uint16_t) */
boolean Adafruit_FONA::getBattVoltage(uint16_t *v) {
  return sendParseReply(F("AT+CBC"), F("+CBC: "), v, ',', 2);
}


/* returns the percentage charge of battery as reported by sim800 */
boolean Adafruit_FONA::getBattPercent(uint16_t *p) {
  return sendParseReply(F("AT+CBC"), F("+CBC: "), p, ',', 1);
}

boolean Adafruit_FONA::getADCVoltage(uint16_t *v) {
  return sendParseReply(F("AT+CADC?"), F("+CADC: 1,"), v);
}

/********* SIM ***********************************************************/

uint8_t Adafruit_FONA::unlockSIM(char *pin)
{
  char sendbuff[14] = "AT+CPIN=";
  sendbuff[8] = pin[0];
  sendbuff[9] = pin[1];
  sendbuff[10] = pin[2];
  sendbuff[11] = pin[3];
  sendbuff[12] = '\0';

  return sendCheckReply(sendbuff, ok_reply);
}

uint8_t Adafruit_FONA::getSIMCCID(char *ccid) {
  getReply(F("AT+CCID"));
  // up to 28 chars for reply, 20 char total ccid
  if (replybuffer[0] == '+') {
    // fona 3g?
    strncpy(ccid, replybuffer+8, 20);
  } else {
    // fona 800 or 800
    strncpy(ccid, replybuffer, 20);
  }
  ccid[20] = 0;

  readline(); // eat 'OK'

  return strlen(ccid);
}

/********* IMEI **********************************************************/

uint8_t Adafruit_FONA::getIMEI(char *imei) {
  getReply(F("AT+GSN"));

  // up to 15 chars
  strncpy(imei, replybuffer, 15);
  imei[15] = 0;

  readline(); // eat 'OK'

  return strlen(imei);
}

/********* NETWORK *******************************************************/

FONA_GET_RSSI Adafruit_FONA::getRSSI(void) {
	
  FONA_GET_RSSI reply;
  uint16_t rssi=0, ber=0, nsPres=0, nsStat=0;
  uint8_t netReg=0;
  
  if (! sendParseReply(F("AT+CSQ"), F("+CSQ: "), &rssi, ',', 0))
	  reply.rssiErr = 1;
	
  if (! sendParseReply(F("AT+CSQ"), F("+CSQ: "), &ber, ',', 1))
	  reply.rssiErr = 1;
  
  if (! sendParseReply(F("AT+CREG?"), F("+CREG: "), &nsPres, ',', 0))
	  netReg |= FONA_GET_NETREG::IS_ERROR;
  else
	  netReg |= nsPres; //Is within GET_NETREG enum bounds
	
  if (! sendParseReply(F("AT+CREG?"), F("+CREG: "), &nsStat, ',', 1))
	  netReg |= FONA_GET_NETREG::IS_ERROR;
  else
	  netReg |= nsStat; //Is within GET_NETREG enum bounds
  
  //Both are <= 99 so can cast&store in 1 byte fields
  reply.rssi = rssi;
  reply.ber = ber;
  
  reply.netReg = (FONA_GET_NETREG)netReg;
  
  return reply;
}


/********* PWM/BUZZER **************************************************/

boolean Adafruit_FONA::setPWM(uint16_t period, uint8_t duty) {
  if (period > 2000)
	return false;
  if (duty > 100)
	return false;

  return sendCheckReply(F("AT+SPWM=0,"), period, duty, ok_reply);
}

/********* SMS **********************************************************/


int8_t Adafruit_FONA::getNumSMS(void) {
  uint16_t numsms;

  // get into text mode
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply))
	return -1;

  // ask how many sms are stored
  if (sendParseReply(F("AT+CPMS?"), F("\"SM\","), &numsms)) 
    return numsms;
  if (sendParseReply(F("AT+CPMS?"), F("\"SM_P\","), &numsms)) 
    return numsms;
	
  return -1;
}

// Reading SMS's is a bit involved so we don't use helpers that may cause delays or debug
// printouts!
boolean Adafruit_FONA::readSMS(uint8_t i, char *smsbuff,
			       uint16_t maxlen, uint16_t *readlen) {
  // text mode
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply))
	return false;

  // show all text mode parameters
  if (! sendCheckReply(F("AT+CSDH=1"), ok_reply))
	return false;

  // parse out the SMS len
  uint16_t thesmslen = 0;

  DEBUG_PRINT(F("AT+CMGR="));
  DEBUG_PRINTLN(i);

  //getReply(F("AT+CMGR="), i, 1000);  //  do not print debug!
  mySerial->print(F("AT+CMGR="));
  mySerial->println(i);
  readline(1000); // timeout

  //DEBUG_PRINT(F("Reply: ")); DEBUG_PRINTLN(replybuffer);
  // parse it out...

  DEBUG_PRINTLN(replybuffer);
  
  if (! parseReply(F("+CMGR:"), &thesmslen, ',', 11)) {
    *readlen = 0;
    return false;
  }

  readRaw(thesmslen);

  flushInput();

  uint16_t thelen = min(maxlen, strlen(replybuffer));
  strncpy(smsbuff, replybuffer, thelen);
  smsbuff[thelen] = 0; // end the string

  DEBUG_PRINTLN(replybuffer);

  *readlen = thelen;
  return true;
}

//TODO:
//"When you make the first device details API call (where no one made the same call in the last 2 minutes), IC2 will trigger the device to send its status data regularly in the coming 2 minutes. So it takes a little bit longer time to receive the first set of data. After that, the system shall receive the data every 2 to 6 secs. The frequency depends on network latency and the system’s load."


//"An update of the RSSI value occurs at the end of each T2 interval. The RSSI value is not valid
//before the AGC settling phase is finished, indicated by RSSI#4 in Figure 1. After this instant
//the gain will not likely be further altered and each update of the RSSI value can be
//considered as valid. The duration from entering RX to the first valid RSSI update is denoted
//RSSI response time"


//TODO: SMS Backup: 
//"As such, users of SMS rarely if ever get a busy or engaged signal as they can do during peak network usage times"


//TODO: Use PDU mode - get it checked from Amir Sb SL


// Retrieve the sender of the specified SMS message and copy it as a string to
// the sender buffer.  Up to senderlen characters of the sender will be copied
// and a null terminator will be added if less than senderlen charactesr are
// copied to the result.  Returns true if a result was successfully retrieved,
// otherwise false.
boolean Adafruit_FONA::getSMSSender(uint8_t i, char *sender, int senderlen) {
	
  // Ensure text mode and all text mode parameters are sent.
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply))
	return false;
	
  if (! sendCheckReply(F("AT+CSDH=1"), ok_reply))
	return false;

  DEBUG_PRINT(F("AT+CMGR="));
  DEBUG_PRINTLN(i);

  // Send command to retrieve SMS message and parse a line of response.
  mySerial->print(F("AT+CMGR="));
  mySerial->println(i);
  readline(1000);

  DEBUG_PRINTLN(replybuffer);

  // Parse the second field in the response.
  boolean result = parseReplyQuoted(F("+CMGR:"), sender, senderlen, ',', 1);
  
  // Drop any remaining data from the response.
  flushInput();
  return result;
}

boolean Adafruit_FONA::sendSMS(char *smsaddr, char *smsmsg) {
	
  if (! sendCheckReply(F("AT+CMGF=1"), ok_reply))
	return false;

  char sendcmd[30] = "AT+CMGS=\"";
  strncpy(sendcmd+9, smsaddr, 30-9-2);  // 9 bytes beginning, 2 bytes for close quote + null
  sendcmd[strlen(sendcmd)] = '\"';

  if (! sendCheckReply(sendcmd, F("> ")))
	return false;

  DEBUG_PRINT(F("> ")); DEBUG_PRINTLN(smsmsg);

  mySerial->println(smsmsg);
  mySerial->println();
  mySerial->write(0x1A);

  DEBUG_PRINTLN("^Z");

  readline(10000); // read the +CMGS reply, wait up to 10 seconds!!!
  //DEBUG_PRINT("Line 3: "); DEBUG_PRINTLN(strlen(replybuffer));
  if (strstr(replybuffer, "+CMGS") == 0) {
    return false;
  }
  readline(1000); // read OK
  //DEBUG_PRINT("* "); DEBUG_PRINTLN(replybuffer);

  if (strcmp(replybuffer, "OK") != 0) {
    return false;
  }

  return true;
}

//TODO: Do we get sms messages stored automatically and hence we need to delete-all occasionally?

boolean Adafruit_FONA::deleteSMS(uint8_t i) {
    if (! sendCheckReply(F("AT+CMGF=1"), ok_reply)) return false;
  // read an sms
  char sendbuff[12] = "AT+CMGD=000";
  sendbuff[8] = (i / 100) + '0';
  i %= 100;
  sendbuff[9] = (i / 10) + '0';
  i %= 10;
  sendbuff[10] = i + '0';

  return sendCheckReply(sendbuff, ok_reply, 2000);
}

/********* USSD *********************************************************/

//use to get carrier info?

boolean Adafruit_FONA::sendUSSD(char *ussdmsg, char *ussdbuff, uint16_t maxlen, uint16_t *readlen) {
  if (! sendCheckReply(F("AT+CUSD=1"), ok_reply)) return false;

  char sendcmd[30] = "AT+CUSD=1,\"";
  strncpy(sendcmd+11, ussdmsg, 30-11-2);  // 11 bytes beginning, 2 bytes for close quote + null
  sendcmd[strlen(sendcmd)] = '\"';

  if (! sendCheckReply(sendcmd, ok_reply)) {
    *readlen = 0;
    return false;
  } else {
      readline(10000); // read the +CUSD reply, wait up to 10 seconds!!!
      //DEBUG_PRINT("* "); DEBUG_PRINTLN(replybuffer);
      char *p = prog_char_strstr(replybuffer, PSTR("+CUSD: "));
      if (p == 0) {
        *readlen = 0;
        return false;
      }
      p+=7; //+CUSD
      // Find " to get start of ussd message.
      p = strchr(p, '\"');
      if (p == 0) {
        *readlen = 0;
        return false;
      }
      p+=1; //"
      // Find " to get end of ussd message.
      char *strend = strchr(p, '\"');

      uint16_t lentocopy = min(maxlen-1, strend - p);
      strncpy(ussdbuff, p, lentocopy+1);
      ussdbuff[lentocopy] = 0;
      *readlen = lentocopy;
  }
  return true;
}

/********* GPS **********************************************************/


boolean Adafruit_FONA::enableGPS(boolean onoff) {
  uint16_t state;

  // first check if its already on or off

  if (! sendParseReply(F("AT+CGNSPWR?"), F("+CGNSPWR: "), &state) )
	return false;

  if (onoff && !state) {
      if (! sendCheckReply(F("AT+CGNSPWR=1"), ok_reply))  // try GNS command
		return false;
  } else if (!onoff && state) {
      if (! sendCheckReply(F("AT+CGNSPWR=0"), ok_reply)) // try GNS command
		return false;
  }
  return true;
}

int8_t Adafruit_FONA::GPSstatus(void) {
	
    // 808 V2 uses GNS commands and doesn't have an explicit 2D/3D fix status.
    // Instead just look for a fix and if found assume it's a 3D fix.
    getReply(F("AT+CGNSINF"));
    char *p = prog_char_strstr(replybuffer, (prog_char*)F("+CGNSINF: "));
    if (p == 0) return -1;
    p+=10;
    readline(); // eat 'OK'
    if (p[0] == '0') return 0; // GPS is not even on!

    p+=2; // Skip to second value, fix status.
    //DEBUG_PRINTLN(p);
    // Assume if the fix status is '1' then we have a 3D fix, otherwise no fix.
    if (p[0] == '1') return 3;
    else return 1;
 
}

uint8_t Adafruit_FONA::getGPS(uint8_t arg, char *buffer, uint8_t maxbuff) {
  int32_t x = arg;

    getReply(F("AT+CGNSINF"));

  char *p = prog_char_strstr(replybuffer, (prog_char*)F("SINF"));
  if (p == 0) {
    buffer[0] = 0;
    return 0;
  }

  p+=6;

  uint8_t len = max(maxbuff-1, strlen(p));
  strncpy(buffer, p, len);
  buffer[len] = 0;

  readline(); // eat 'OK'
  return len;
}

boolean Adafruit_FONA::getGPS(
	float *lat, float *lon, float *speed_kph, float *heading, float *altitude, char *date) {

  char gpsbuffer[120];

  // we need at least a 2D fix
  if (GPSstatus() < 2)
    return false;

  // grab the mode 2^5 gps csv from the sim808
  uint8_t res_len = getGPS(32, gpsbuffer, 120);

  // make sure we have a response
  if (res_len == 0)
    return false;

	//								mySerial->print(gpsbuffer);

    // Parse 808 V2 response.  See table 2-3 from here for format:
    // http://www.adafruit.com/datasheets/SIM800%20Series_GNSS_Application%20Note%20V1.00.pdf

    // skip GPS run status
    char *tok = strtok(gpsbuffer, ",");
    if (! tok) return false;

    // skip fix status
    tok = strtok(NULL, ",");
    if (! tok) return false;

    // grab date
    char *datep = strtok(NULL, ",");
    if (! datep) return false;

    // grab the latitude
    char *latp = strtok(NULL, ",");
    if (! latp) return false;

    // grab longitude
    char *longp = strtok(NULL, ",");
    if (! longp) return false;

	strncpy(date, datep, 14); //yyyyMMddHHmmss
	date[14] = 0;
    *lat = atof(latp);
    *lon = atof(longp);

    // only grab altitude if needed
    if (altitude != NULL) {
      // grab altitude
      char *altp = strtok(NULL, ",");
      if (! altp) return false;

      *altitude = atof(altp);
    }

    // only grab speed if needed
    if (speed_kph != NULL) {
      // grab the speed in km/h
      char *speedp = strtok(NULL, ",");
      if (! speedp) return false;

      *speed_kph = atof(speedp);
    }

    // only grab heading if needed
    if (heading != NULL) {

      // grab the speed in knots
      char *coursep = strtok(NULL, ",");
      if (! coursep) return false;

      *heading = atof(coursep);
    }
 
  return true;

}

boolean Adafruit_FONA::enableGPSNMEA(uint8_t i) {

  char sendbuff[15] = "AT+CGPSOUT=000";
  sendbuff[11] = (i / 100) + '0';
  i %= 100;
  sendbuff[12] = (i / 10) + '0';
  i %= 10;
  sendbuff[13] = i + '0';

    if (i)
      return sendCheckReply(F("AT+CGNSTST=1"), ok_reply);
    else
      return sendCheckReply(F("AT+CGNSTST=0"), ok_reply);
}


/********* GPRS **********************************************************/


FONA_STATUS_GPRS_INIT Adafruit_FONA::enableGPRS(boolean onoff) {

  FONA_STATUS_GPRS_INIT ret = FONA_STATUS_GPRS_INIT::SUCCESS_FSGI;
  
  if (onoff) {
	  
    // disconnect all sockets
    if (!sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 65000))
		ret = FONA_STATUS_GPRS_INIT::WARN_CIPSHUT_BEFORE_OPENING;

    if (!sendCheckReply(F("AT+CGATT=1"), ok_reply, 10000))
	  return FONA_STATUS_GPRS_INIT::ERR_CGATT_ATTACH; 

    // set bearer profile! connection type GPRS
    if (! sendCheckReply(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), ok_reply, 10000))
	  return FONA_STATUS_GPRS_INIT::ERR_SAPBR_SET_CONN_TYPE;

    // set bearer profile access point name
    if (apn) {
      // Send command AT+SAPBR=3,1,"APN","<apn value>" where <apn value> is the configured APN value.
      if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"APN\","), apn, ok_reply, 10000))
		  return FONA_STATUS_GPRS_INIT::ERR_SAPBR_SET_APN;

      // send AT+CSTT,"apn","user","pass"
      flushInput();

      mySerial->print(F("AT+CSTT=\""));
      mySerial->print(apn);
      if (apnusername) {
		mySerial->print("\",\"");
		mySerial->print(apnusername);
      }
      if (apnpassword) {
		mySerial->print("\",\"");
		mySerial->print(apnpassword);
      }
      mySerial->println("\"");

      DEBUG_PRINT(F("\t---> ")); DEBUG_PRINT(F("AT+CSTT=\""));
      DEBUG_PRINT(apn); 
      
      if (apnusername) {
		DEBUG_PRINT("\",\"");
		DEBUG_PRINT(apnusername); 
      }
      if (apnpassword) {
		DEBUG_PRINT("\",\"");
		DEBUG_PRINT(apnpassword);
      }
      DEBUG_PRINTLN("\"");
      
      if (! expectReply(ok_reply))
	      return FONA_STATUS_GPRS_INIT::ERR_SAPBR_START_APN_TASK;
    
      // set username/password
      if (apnusername) {
		  
        // Send command AT+SAPBR=3,1,"USER","<user>" where <user> is the configured APN username.
        if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"USER\","), apnusername, ok_reply, 10000))
		  return FONA_STATUS_GPRS_INIT::ERR_SAPBR_SET_USER;
      }
      if (apnpassword) {
		  
        // Send command AT+SAPBR=3,1,"PWD","<password>" where <password> is the configured APN password.
        if (! sendCheckReplyQuoted(F("AT+SAPBR=3,1,\"PWD\","), apnpassword, ok_reply, 10000))
          return FONA_STATUS_GPRS_INIT::ERR_SAPBR_SET_PWD;
      }
    }

    // open GPRS context
    if (! sendCheckReply(F("AT+SAPBR=1,1"), ok_reply, 65535)) //TODO: Should be 85000 but not supported by this data-type
      return FONA_STATUS_GPRS_INIT::ERR_SAPBR_WHEN_OPENING;

    // bring up wireless connection
    if (! sendCheckReply(F("AT+CIICR"), ok_reply, 65535)) //TODO: Should be 85000 but not supported by this data-type
      return FONA_STATUS_GPRS_INIT::ERR_CIICR_WHEN_ON_WIRELESS;

  } else {
	  
    // disconnect all sockets
    if (! sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 65000))
      return FONA_STATUS_GPRS_INIT::ERR_CIPSHUT_CLOSE_SOCKET;

    // close GPRS context
    if (! sendCheckReply(F("AT+SAPBR=0,1"), ok_reply, 10000))
		return FONA_STATUS_GPRS_INIT::ERR_SAPBR_CLOSE_GPRS;

    if (! sendCheckReply(F("AT+CGATT=0"), ok_reply, 10000))
		return FONA_STATUS_GPRS_INIT::ERR_CGATT_DETACH;
  }
  
  return ret;
}


uint8_t Adafruit_FONA::GPRSstate(void) {
  uint16_t state;

  if (! sendParseReply(F("AT+CGATT?"), F("+CGATT: "), &state) )
    return -1;

  return state;
}

void Adafruit_FONA::setGPRSNetworkSettings(FONAFlashStringPtr apn,
              FONAFlashStringPtr username, FONAFlashStringPtr password) {
  this->apn = apn;
  this->apnusername = username;
  this->apnpassword = password;
}

boolean Adafruit_FONA::getGSMLoc(uint16_t *errorcode, char *buff, uint16_t maxlen) {

  getReply(F("AT+CIPGSMLOC=1,1"), (uint16_t)60000);

  if (! parseReply(F("+CIPGSMLOC: "), errorcode))
    return false;

  char *p = replybuffer+14;
  uint16_t lentocopy = min(maxlen-1, strlen(p));
  strncpy(buff, p, lentocopy+1);

  readline(); // eat OK

  return true;
}

boolean Adafruit_FONA::getGSMLoc(float *lat, float *lon) {

  uint16_t returncode;
  char gpsbuffer[120];

  // make sure we could get a response
  if (! getGSMLoc(&returncode, gpsbuffer, 120))
    return false;

  // make sure we have a valid return code
  if (returncode != 0)
    return false;

  // +CIPGSMLOC: 0,-74.007729,40.730160,2015/10/15,19:24:55
  // tokenize the gps buffer to locate the lat & long
  char *longp = strtok(gpsbuffer, ",");
  if (! longp) return false;

  char *latp = strtok(NULL, ",");
  if (! latp) return false;

  *lat = atof(latp);
  *lon = atof(longp);

  return true;

}
/********* TCP FUNCTIONS  ************************************/
/* Not required for RMonV3 - all removed */


/********* HTTP LOW LEVEL FUNCTIONS  ************************************/

boolean Adafruit_FONA::HTTP_init() {
  return sendCheckReply(F("AT+HTTPINIT"), ok_reply);
}

boolean Adafruit_FONA::HTTP_term() {
  return sendCheckReply(F("AT+HTTPTERM"), ok_reply);
}

void Adafruit_FONA::HTTP_para_start(FONAFlashStringPtr parameter,
                                    boolean quoted) {
  flushInput();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+HTTPPARA=\""));
  DEBUG_PRINT(parameter);
  DEBUG_PRINTLN('"');


  mySerial->print(F("AT+HTTPPARA=\""));
  mySerial->print(parameter);
  if (quoted)
    mySerial->print(F("\",\""));
  else
    mySerial->print(F("\","));
}

boolean Adafruit_FONA::HTTP_para_end(boolean quoted) {
  if (quoted)
    mySerial->println('"');
  else
    mySerial->println();

  return expectReply(ok_reply);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 const char *value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
								const String value) {
	HTTP_para_start(parameter, true);
	mySerial->print(value);
	return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 FONAFlashStringPtr value) {
  HTTP_para_start(parameter, true);
  mySerial->print(value);
  return HTTP_para_end(true);
}

boolean Adafruit_FONA::HTTP_para(FONAFlashStringPtr parameter,
                                 int32_t value) {
  HTTP_para_start(parameter, false);
  mySerial->print(value);
  return HTTP_para_end(false);
}

boolean Adafruit_FONA::HTTP_data(uint32_t size, uint32_t maxTime) {
  flushInput();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(F("AT+HTTPDATA="));
  DEBUG_PRINT(size);
  DEBUG_PRINT(',');
  DEBUG_PRINTLN(maxTime);


  mySerial->print(F("AT+HTTPDATA="));
  mySerial->print(size);
  mySerial->print(",");
  mySerial->println(maxTime);

  return expectReply(F("DOWNLOAD"));
}

boolean Adafruit_FONA::HTTP_action(uint8_t method, uint16_t *status,
                                   uint16_t *datalen, int32_t timeout) {
  // Send request.
  if (! sendCheckReply(F("AT+HTTPACTION="), method, ok_reply))
    return false;

  // Parse response status and size.
  readline(timeout);
  if (! parseReply(F("+HTTPACTION:"), status, ',', 1))
    return false;
  if (! parseReply(F("+HTTPACTION:"), datalen, ',', 2))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_readall(uint16_t *datalen) {
  getReply(F("AT+HTTPREAD"));
  if (! parseReply(F("+HTTPREAD:"), datalen, ',', 0))
    return false;

  return true;
}

boolean Adafruit_FONA::HTTP_ssl(boolean onoff) {
  return sendCheckReply(F("AT+HTTPSSL="), onoff ? 1 : 0, ok_reply);
}

/********* HTTP HIGH LEVEL FUNCTIONS ***************************/

boolean Adafruit_FONA::HTTP_GET_start(char *url,
              uint16_t *status, uint16_t *datalen){
				  
  if (! HTTP_setup(url))
    return false;

  // HTTP GET
  if (! HTTP_action(FONA_HTTP_GET, status, datalen, 30000))
    return false;

  DEBUG_PRINT(F("Status: "));
  DEBUG_PRINTLN(*status);
  DEBUG_PRINT(F("Len: "));
  DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

void Adafruit_FONA::HTTP_GET_end(void) {
  HTTP_term();
}

boolean Adafruit_FONA::HTTP_POST_start(char *url,
              FONAFlashStringPtr contenttype,
              const uint8_t *postdata, uint16_t postdatalen,
              uint16_t *status, uint16_t *datalen){
				  
  if (! HTTP_setup(url))
    return false;

  if (! HTTP_para(F("CONTENT"), contenttype)) {
    return false;
  }

  // HTTP POST data
  if (! HTTP_data(postdatalen, 10000))
    return false;
	
  mySerial->write(postdata, postdatalen);
  
  if (! expectReply(ok_reply))
    return false;

  // HTTP POST
  if (! HTTP_action(FONA_HTTP_POST, status, datalen))
    return false;

  DEBUG_PRINT(F("Status: "));
  DEBUG_PRINTLN(*status);
  DEBUG_PRINT(F("Len: "));
  DEBUG_PRINTLN(*datalen);

  // HTTP response data
  if (! HTTP_readall(datalen))
    return false;

  return true;
}

void Adafruit_FONA::HTTP_POST_end(void) {
  HTTP_term();
}

void Adafruit_FONA::setUserAgent(String useragent) {
  this->useragent = useragent;
}

void Adafruit_FONA::setHTTPSRedirect(boolean onoff) {
  httpsredirect = onoff;
}

/********* HTTP HELPERS ****************************************/

boolean Adafruit_FONA::HTTP_setup(char *url) {
  // Handle any pending
  HTTP_term();

  // Initialize and set parameters
  if (! HTTP_init())
    return false;
  if (! HTTP_para(F("CID"), 1))
    return false;
  if (! HTTP_para(F("UA"), useragent))
    return false;
  if (! HTTP_para(F("URL"), url))
    return false;

  // HTTPS redirect
  if (httpsredirect) {
    if (! HTTP_para(F("REDIR"),1))
      return false; //Make it just a warning

    if (! HTTP_ssl(true))
      return false;
  }
  
  
  //TODO: BREAK and BREAKEND !!
  
  

  return true;
}

/********* HELPERS *********************************************/

boolean Adafruit_FONA::expectReply(FONAFlashStringPtr reply,
                                   uint16_t timeout) {
  readline(timeout);

  DEBUG_PRINT(F("\t<--- "));
  DEBUG_PRINTLN(replybuffer);

  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

/********* LOW LEVEL *******************************************/

inline int Adafruit_FONA::available(void) {
  return mySerial->available();
}

inline size_t Adafruit_FONA::write(uint8_t x) {
  return mySerial->write(x);
}

inline int Adafruit_FONA::read(void) {
  return mySerial->read();
}

inline int Adafruit_FONA::peek(void) {
  return mySerial->peek();
}

inline void Adafruit_FONA::flush() {
  mySerial->flush();
}

void Adafruit_FONA::flushInput() {
    // Read all available serial input to flush pending data.
    uint16_t timeoutloop = 0;
    while (timeoutloop++ < 40) {
        while(available()) {
            read();
            timeoutloop = 0;  // If char was received reset the timer
        }
        delay(1);
    }
}

uint16_t Adafruit_FONA::readRaw(uint16_t b) {
  uint16_t idx = 0;

  while (b && (idx < sizeof(replybuffer)-1)) {
    if (mySerial->available()) {
      replybuffer[idx] = mySerial->read();
      idx++;
      b--;
    }
  }
  replybuffer[idx] = 0;

  return idx;
}

uint8_t Adafruit_FONA::readline(uint16_t timeout, boolean multiline) {
  uint16_t replyidx = 0;

  timeout *= 4;
  
  while (timeout--) {
    if (replyidx >= 254) {
      DEBUG_PRINTLN(F("SPACE"));
      break;
    }

    while(mySerial->available()) {
      char c =  mySerial->read();
      if (c == '\r') continue;
      if (c == 0xA) {
        if (replyidx == 0)   // the first 0x0A is ignored
          continue;

        if (!multiline) {
          timeout = 0;         // the second 0x0A is the end of the line
          break;
        }
      }
      replybuffer[replyidx] = c;
      //DEBUG_PRINT(c, HEX); DEBUG_PRINT("#"); DEBUG_PRINTLN(c);
      replyidx++;
    }

	//TODO: if (argumentTimeout>0 && timeout==0){...
    if (timeout == 0) {
     // DEBUG_PRINTLN(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  replybuffer[replyidx] = 0;  // null term
  return replyidx;
}

uint8_t Adafruit_FONA::getReply(char *send, uint16_t timeout) {
	
  flushInput();


  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINTLN(send);

  mySerial->println(send);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- "));
  DEBUG_PRINTLN(replybuffer);

  return l;
}

uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr send, uint16_t timeout) {
	
  flushInput();

  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINTLN(send);

  mySerial->println(send);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- "));
  DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, char *suffix, uint16_t timeout) {
	
  flushInput();

  DEBUG_PRINT(F("\t---> ")); 
  DEBUG_PRINT(prefix); 
  DEBUG_PRINTLN(suffix);

  mySerial->print(prefix);
  mySerial->println(suffix);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- "));
  DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, int32_t suffix, uint16_t timeout) {
	
  flushInput();

  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(prefix);
  DEBUG_PRINTLN(suffix, DEC);

  mySerial->print(prefix);
  mySerial->println(suffix, DEC);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- "));
  DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, suffix, suffix2, and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReply(FONAFlashStringPtr prefix, int32_t suffix1, int32_t suffix2, uint16_t timeout) {

  flushInput();

  DEBUG_PRINT(F("\t---> ")); 
  DEBUG_PRINT(prefix);
  DEBUG_PRINT(suffix1, DEC); 
  DEBUG_PRINT(','); 
  DEBUG_PRINTLN(suffix2, DEC);

  mySerial->print(prefix);
  mySerial->print(suffix1);
  mySerial->print(',');
  mySerial->println(suffix2, DEC);

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- "));
  DEBUG_PRINTLN(replybuffer);

  return l;
}

// Send prefix, ", suffix, ", and newline. Return response (and also set replybuffer with response).
uint8_t Adafruit_FONA::getReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, uint16_t timeout) {
  
  flushInput();

  DEBUG_PRINT(F("\t---> "));
  DEBUG_PRINT(prefix);
  DEBUG_PRINT('"');
  DEBUG_PRINT(suffix);
  DEBUG_PRINTLN('"');

  mySerial->print(prefix);
  mySerial->print('"');
  mySerial->print(suffix);
  mySerial->println('"');

  uint8_t l = readline(timeout);

  DEBUG_PRINT (F("\t<--- "));
  DEBUG_PRINTLN(replybuffer);

  return l;
}

boolean Adafruit_FONA::sendCheckReply(char *send, char *reply, uint16_t timeout) {
	
  if (! getReply(send, timeout) )
	  return false;
/*
  for (uint8_t i=0; i<strlen(replybuffer); i++) {
  DEBUG_PRINT(replybuffer[i], HEX); DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  for (uint8_t i=0; i<strlen(reply); i++) {
    DEBUG_PRINT(reply[i], HEX); DEBUG_PRINT(" ");
  }
  DEBUG_PRINTLN();
  */
  return (strcmp(replybuffer, reply) == 0);
}

boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr send, FONAFlashStringPtr reply, uint16_t timeout) {

	if (! getReply(send, timeout) )
		return false;

  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

boolean Adafruit_FONA::sendCheckReply(char* send, FONAFlashStringPtr reply, uint16_t timeout) {
	
  if (! getReply(send, timeout) )
	  return false;
	  
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}


// Send prefix, suffix, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, char *suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, suffix, suffix2, and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReply(FONAFlashStringPtr prefix, int32_t suffix1, int32_t suffix2, FONAFlashStringPtr reply, uint16_t timeout) {
  getReply(prefix, suffix1, suffix2, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}

// Send prefix, ", suffix, ", and newline.  Verify FONA response matches reply parameter.
boolean Adafruit_FONA::sendCheckReplyQuoted(FONAFlashStringPtr prefix, FONAFlashStringPtr suffix, FONAFlashStringPtr reply, uint16_t timeout) {
  getReplyQuoted(prefix, suffix, timeout);
  return (prog_char_strcmp(replybuffer, (prog_char*)reply) == 0);
}


boolean Adafruit_FONA::parseReply(FONAFlashStringPtr toreply,
          uint16_t *v, char divider, uint8_t index) {
			  
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);  // get the pointer to the voltage
  if (p == 0)
	return false;
  
  p+=prog_char_strlen((prog_char*)toreply);
  
  //DEBUG_PRINTLN(p);
  for (uint8_t i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p)
		return false;
    p++;
    //DEBUG_PRINTLN(p);

  }
  
  *v = atoi(p);

  return true;
}

boolean Adafruit_FONA::parseReply(FONAFlashStringPtr toreply,
          char *v, char divider, uint8_t index) {
			  
  uint8_t i=0;
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);
  if (p == 0)
	return false;
	
  p+=prog_char_strlen((prog_char*)toreply);

  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p)
		return false;
    p++;
  }

  for(i=0; i<strlen(p);i++) {
    if(p[i] == divider)
      break;
    v[i] = p[i];
  }

  v[i] = '\0';

  return true;
}

// Parse a quoted string in the response fields and copy its value (without quotes)
// to the specified character array (v).  Only up to maxlen characters are copied
// into the result buffer, so make sure to pass a large enough buffer to handle the
// response.
boolean Adafruit_FONA::parseReplyQuoted(FONAFlashStringPtr toreply,
          char *v, int maxlen, char divider, uint8_t index) {
			  
  uint8_t i=0, j;
  // Verify response starts with toreply.
  char *p = prog_char_strstr(replybuffer, (prog_char*)toreply);
  if (p == 0)
	return false;
	
  p+=prog_char_strlen((prog_char*)toreply);

  // Find location of desired response field.
  for (i=0; i<index;i++) {
    // increment dividers
    p = strchr(p, divider);
    if (!p)
		return false;
    p++;
  }

  // Copy characters from response field into result string.
  for(i=0, j=0; j<maxlen && i<strlen(p); ++i) {
    // Stop if a divier is found.
    if(p[i] == divider)
      break;
    // Skip any quotation marks.
    else if(p[i] == '"')
      continue;
    v[j++] = p[i];
  }

  // Add a null terminator if result string buffer was not filled.
  if (j < maxlen)
    v[j] = '\0';

  return true;
}

boolean Adafruit_FONA::sendParseReply(FONAFlashStringPtr tosend,
				      FONAFlashStringPtr toreply,
				      uint16_t *v, char divider, uint8_t index) {
						
  getReply(tosend);

  if (! parseReply(toreply, v, divider, index)) {
	return false;
  }

  readline(); // eat 'OK'

  return true;
}


/***************************************************

  This is a library for our Adafruit FONA Cellular Module

  Designed specifically to work with the Adafruit FONA
  ----> http://www.adafruit.com/products/1946
  ----> http://www.adafruit.com/products/1963

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
  
  Trimmed for RMonV3 @ 21/5/20
  
 ****************************************************/
