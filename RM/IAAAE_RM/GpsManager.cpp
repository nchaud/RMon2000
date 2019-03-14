#include <Arduino.h>
#include "Helpers.h"
#include "Adafruit_FONA.h"
#include "GpsManager.h"

GpsManager::GpsManager(uint8_t isMock)
					  :_isMock(isMock)
{
}

GpsManager::~GpsManager(){}
	
void GpsManager::setFona(Adafruit_FONA& fonaInstance){
	fona = fonaInstance;
}

boolean GpsManager::toggleGps(boolean onOff){
	
	if (_isMock)
		return true;
	
	int var = 99999;
	
	//This does what?
	//if (onOff)
		//return fona.enableGPSNMEA(var);
	//
	//return fona.enableGPS(onOff);
}

int8_t GpsManager::gpsStatus(){
	
	//if (_isMock)
		//return 3; //Magic
	//
	//return fona.GPSstatus();
}

void GpsManager::getGpsInfo(GpsInfo& info){

	if (_isMock)
	{
		info.altitude = 10;
		
		strcpy(info.date, "20180301181716");
		//info.date[14]='\0';
		
		info.gpsStatus = 30;
		info.heading = 40;
		info.lat = 50;
		info.lon = 60;
		info.speed_kph = 70;
		return;
	}

	//int8_t gpsStatus = fona.GPSstatus();
	//
	////Store the status regardless
	//info.gpsStatus = gpsStatus;
	//
	//// we need at least a 2D fix
	//if (gpsStatus < 2) {
		//info.errorCode = ERR_GPS_NO_FIX;
		//return;
	//}
//
	////We know the date won't be >20 from it's format
	//boolean success = fona.getGPS(&info.lat, &info.lon, 
		//&info.speed_kph, &info.heading, &info.altitude, (char*)info.date);
//
	//// make sure we have a response
	//if (!success){
		//info.errorCode = ERR_GPS_BAD_FIELD;
		//return;
	//}
}



////Once a week, refresh GPS
//if (_isWeeklyCycle) {
	//
	//triggerGpsRefreshAsync();
	//
	//gpsData = GpsData(); //Reset GPS field
	//_gpsFetchInProgress = true;
	//
	//doContinueCycle |= true;
//}
//
////Every 30 seconds, check if GPS is ready
//if (_gpsFetchInProgress) {
//
	//bool gpsSuccess = 0;
	//if (_has5MinElapsed) {
		//
		//gpsData = getGpsSensorData();
//
		//If successfull (2D or 3D fix), save down
		//if (gpsData.success)
		  //saveDataFromGps(gpsData);
		//
		//Attempt to get GPS-3D-Fix for 5 mins, else fail
		//bool gpsDone = gpsData.is3DFix || _has5MinElapsed;
		//
		//Shut down GPS module as consumes battery
		//if (gpsDone) {
		//#ifdef DEBUG
		//onGpsComplete();
		//#endif
//
		//RM_LOG2(F("GSM RawData77"),gpsData.raw);
		//if (gpsData.success)
		//execTransmitGps();
		 //}
		//
		//_gpsFetchInProgress = false;//!gpsDone;
		//doContinueCycle |=    false;//!gpsDone;
	//}
	//else
	//doContinueCycle |= true;
//}
//#endif

/****************************************************/
/********************** GPS *************************/
/****************************************************/

//void loadDataForGps(char* buffer, int maxSize) {

	//char* curr = buffer;
//
	//RM_LOG2(F("Buffer"), buffer);
	//RM_LOG2(F("Raw Data"), gpsData.raw);
	//RM_LOG2(F("Max Sz"), maxSize);
	//RM_LOG2(F("Loc Data"), gpsData.gsmLoc);
	//RM_LOG2(F("Raw Sz"), gpsData.rawSz);
	//RM_LOG2(F("Loc Sz"), gpsData.gsmLocSz);
	//
//	uint8_t status = fona.GPSstatus();
	//
////	Header to indicate data type
	//if (gpsData.gsmLocSz == 0 && gpsData.rawSz == 0) {
		//sprintf(curr++, "%s", "X");
	//}
	//else if (gpsData.gsmLocSz == 0) {
		//sprintf(curr++, "%s", "P"); //gPs format
		//
		//strncpy(curr,gpsData.raw,gpsData.rawSz);
		//curr += gpsData.rawSz;
	//}
	//else if (gpsData.rawSz == 0) {
		//sprintf(curr++, "%s", "S");  //gSm format
		//
		//strncpy(curr,gpsData.gsmLoc,gpsData.gsmLocSz);
		//curr += gpsData.gsmLocSz;
	//}
	//else{
	////	Both valid, choose one randomly
		//
		//if (millis()%2==0) {
			 //sprintf(curr++, "%s", "T");
			//
			//char local[gpsData.rawSz+1];
			//strncpy(local, gpsData.raw, gpsData.rawSz);
			//local[gpsData.rawSz]=0;
			//
			//strncpy(curr, local, gpsData.rawSz);
		//}
		//else {
			   //sprintf(curr++, "%s", "U");
			//
			//char local[gpsData.gsmLocSz+1];
			//strncpy(local, gpsData.gsmLoc, gpsData.gsmLocSz);
			//local[gpsData.gsmLocSz]=0;
			//
			//strncpy(curr, local, gpsData.gsmLocSz);
		//}
	//}
//
////	This gets truncated for some reason
	//RM_LOG2(F("Buffer2"), buffer);
	//RM_LOG2(F("Raw Data22"), gpsData.raw);
	 //RM_LOG2(F("Loc Data22"), gpsData.gsmLoc);
	 //RM_LOG2(F("Raw Sz22"), gpsData.rawSz);
	//RM_LOG2(F("Loc Sz22"), gpsData.gsmLocSz);
//}

//void triggerGpsRefreshAsync() {
//
	//RM_LOG(F("GPS Switching On"));
	//if (!fona.enableGPS(true))
		//addError("G-EB");
	//
	//if (!fona.enableGPRS(true)) //We can use GSM-LOC to give us location data via GSM network!
		//addError("P-EB");
//}

//void /*GpsData */ getGpsSensorData() {

	//GpsData ret;
//
	//RM_LOG(F("GPS Getting Loc Data"));
	//// check for GSMLOC (requires GPRS)
	//boolean gsmLocSuccess = false;
	//{
		//uint16_t returncode;
		//
		//if (fona.getGSMLoc(&returncode, replybuffer, 250)) {
			//if (returncode == 0) {
//
			////	TODO: Can I even do this - storing local to global?
				//char gsmLocData[50];
				//strncpy(gsmLocData,replybuffer, 50);
				//ret.gsmLoc = gsmLocData;
				//ret.gsmLocSz = sizeof(gsmLocData);
				//RM_LOG2(F("GSM Orig.LocData"),ret.gsmLoc);
				//RM_LOG2(F("GSM Orig.LocDataSz"),ret.gsmLocSz);
				//gsmLocSuccess = true;
			//}
		//}
		//else
		//returncode=9; //our own :)
		//
		//ret.gsmErrCode = returncode;
	//}
	//
	//// check GPS fix
	//int8_t stat;
	//stat = fona.GPSstatus();
	//boolean isGpsSuccess = false;
	//
	//if (stat < 0)
	//addError(new char[3]{'G','S',stat}); //test this concat
	//else {
		//
		//if (stat > 1){ //If we have a fix
			//isGpsSuccess = true;
			//ret.is3DFix = stat == 3;
//
			//char gpsdataArr[120];
			//fona.getGPS(0, gpsdataArr, 120);
//
			////TODO: Can I even do this - storing local to global?
			//ret.raw = gpsdataArr;
			//ret.rawSz = sizeof(gpsdataArr);
			//
			//RM_LOG2(F("GPS Retrieved-Full"),ret.raw);
		//}
	//}
//
	//ret.success = gsmLocSuccess || isGpsSuccess;
//
	//return ret;
//}

//void execTransmitGps() {
//
	 //RM_LOG2(F("FreeMemory"), get_free_memory());
	 //RM_LOG2(F("GSM RawData2"),gpsData.raw);
	//
	//char netBuffer[120+12]; //MAX{GPS-Size, Loc-Size} + Header
	//char* currPos = initBufferForSend(netBuffer, 0);
	//RM_LOG2(F("Headers Prepped"), netBuffer);
	//loadDataForGps(currPos, sizeof(netBuffer)-(currPos-netBuffer));
	//RM_LOG2(F("Sending GPS via GPRS2"), netBuffer);
	////Try GPRS, else SMS
	//uint16_t fCode = sendViaGprs(netBuffer);
	//
	//if (fCode > 0) {
		//RM_LOG2(F("GPRS Failed-Trying SMS..."),fCode);
		//char smsBuffer[140]; //SMS limit
		//
		//currPos = initBufferForSend(smsBuffer, fCode);
		//loadDataForGps(currPos, sizeof(smsBuffer)-(currPos-smsBuffer));
		//
		//if (!sendViaSms(netBuffer)) {
			//RM_LOG(F("SMS Send Failed !!"));
		//}
	//}
//}
//

//void onGpsComplete() {
	//RM_LOG(F("GPS Shutdown"));
	//fona.enableGPS(false);
	//fona.enableGPRS(false); //TODO: Here ?!
//}


//void loadDataForReadings_OLD(char* buffer, int maxSize) {
//
	//char* curr = buffer;
	//
	//Header to indicate data type
	//sprintf(curr, "%s", "R");
	//curr+=1;
//
	//maxSize-=3;
//
	//uint8_t readingSize = 7*3;
	//uint8_t maxNoOfReadings = maxSize/readingSize;
	//uint8_t totalNoOfReadings = sizeof(sensorDataArr)/sizeof(SensorData);
//
	//RM_LOG2(F("Max Sz"), maxSize);
	//RM_LOG2(F("maxNoOfReadings"), maxNoOfReadings);
	//RM_LOG2(F("totalNoOfReadings"), totalNoOfReadings);
	//
	//if (maxNoOfReadings == 0) //No space for any readings :|
	//return;
//
	//uint8_t jumpValue = totalNoOfReadings/maxNoOfReadings;
	//if (jumpValue==0) //i.e. more space than available readings
	//jumpValue=1;
	//
	//for(uint8_t readingIdx=0 ; readingIdx<totalNoOfReadings ; readingIdx += jumpValue) {
//
		//RM_LOG2(F("FreeMemory"), get_free_memory());
		//RM_LOG2(F("Reading Idx"), readingIdx);
		//SensorData reading = sensorDataArr[readingIdx];
		//
		//Copy Sensor data in
		//char battChar[8], pvChar[8], currChar[8];
		//
		//dtostrf (reading.VBatt,7,3, battChar); //102.345 Volts, for example
		//strncpy(curr,battChar,7);
		//
		//dtostrf (reading.VPV,7,3, pvChar);
		//strncpy(curr+=7,pvChar,7);
		//
		//dtostrf (reading.Current,7,3, currChar);
		//strncpy(curr+=7,currChar,7);
		//
		//curr+=7;
		//If buffers change, modify readingSize variable too
	//}
//}
