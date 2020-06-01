#include <Arduino.h>
#include "DataTypes.h"
#include "Helpers.h"

void Helpers::printRSSI(FONA_GET_RSSI* rssi) {
	
	RM_LOG(F("RSSI="));
	RM_LOG(rssi->rssi);
	RM_LOG(F(" | "));
	
	RM_LOG(F("BER="));
	RM_LOG(rssi->ber);
	RM_LOG(F(" | "));
	
	RM_LOG(F("Network-Reg ResCode="));
	RM_LOG(NETREG_ACTUALVAL_RESULT_CODE(rssi->netReg));
	RM_LOG(F(", Status="));
	RM_LOG(NETREG_ACTUALVAL_NETSTAT(rssi->netReg));
	RM_LOG(F(", Error="));
	RM_LOG(NETREG_ACTUALVAL_ERROR(rssi->netReg));
	RM_LOG(F(" | "));
	
	RM_LOG(F("RSSI Error?="));
	RM_LOGLN(rssi->rssiErr);
}

void Helpers::printSensorData(SensorData* sd) {

	RM_LOG(F("Batt-V="));
	RM_LOG(sd->battVoltage);
	RM_LOG(F(" | "));
	
	RM_LOG(F("PV-V="));
	RM_LOG(sd->pVVoltage);
	RM_LOG(F(" | "));
	
	RM_LOG(F("Current="));
	RM_LOG(sd->current);
	RM_LOG(F(" | "));
	
	RM_LOG(F("Temp="));
	RM_LOGLN(sd->temperature);
}


const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

/* 'Private' declarations */
inline void a3_to_a4(unsigned char * a4, unsigned char * a3);
inline void a4_to_a3(unsigned char * a3, unsigned char * a4);
inline unsigned char b64_lookup(char c);

/* Note: Trailing 0s for strings shouldn't be included in the length */
int Helpers::base64_encode(char *output, char *input, int inputLen) {
	int i = 0, j = 0;
	int encLen = 0;
	unsigned char a3[3];
	unsigned char a4[4];

	while(inputLen--) {
		a3[i++] = *(input++);
		if(i == 3) {
			a3_to_a4(a4, a3);

			for(i = 0; i < 4; i++) {
				output[encLen++] = pgm_read_byte(&b64_alphabet[a4[i]]);
			}

			i = 0;
		}
	}

	if(i) {
		for(j = i; j < 3; j++) {
			a3[j] = '\0';
		}

		a3_to_a4(a4, a3);

		for(j = 0; j < i + 1; j++) {
			output[encLen++] = pgm_read_byte(&b64_alphabet[a4[j]]);
		}

		while((i++ < 3)) {
			output[encLen++] = '=';
		}
	}
	output[encLen] = '\0';
	return encLen;
}

/* Note: Trailing 0s for strings shouldn't be included in the length */
int Helpers::base64_decode(char * output, char * input, int inputLen) {
	int i = 0, j = 0;
	int decLen = 0;
	unsigned char a3[3];
	unsigned char a4[4];


	while (inputLen--) {
		if(*input == '=') {
			break;
		}

		a4[i++] = *(input++);
		if (i == 4) {
			for (i = 0; i <4; i++) {
				a4[i] = b64_lookup(a4[i]);
			}

			a4_to_a3(a3,a4);

			for (i = 0; i < 3; i++) {
				output[decLen++] = a3[i];
			}
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++) {
			a4[j] = '\0';
		}

		for (j = 0; j <4; j++) {
			a4[j] = b64_lookup(a4[j]);
		}

		a4_to_a3(a3,a4);

		for (j = 0; j < i - 1; j++) {
			output[decLen++] = a3[j];
		}
	}
	output[decLen] = '\0';
	return decLen;
}

int Helpers::base64_enc_len(int plainLen) {
	int n = plainLen;
	return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

int Helpers::base64_dec_len(char * input, int inputLen) {
	int i = 0;
	int numEq = 0;
	for(i = inputLen - 1; input[i] == '='; i--) {
		numEq++;
	}

	return ((6 * inputLen) / 8) - numEq;
}

inline void a3_to_a4(unsigned char * a4, unsigned char * a3) {
	a4[0] = (a3[0] & 0xfc) >> 2;
	a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
	a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
	a4[3] = (a3[2] & 0x3f);
}

inline void a4_to_a3(unsigned char * a3, unsigned char * a4) {
	a3[0] = (a4[0] << 2) + ((a4[1] & 0x30) >> 4);
	a3[1] = ((a4[1] & 0xf) << 4) + ((a4[2] & 0x3c) >> 2);
	a3[2] = ((a4[2] & 0x3) << 6) + a4[3];
}

inline unsigned char b64_lookup(char c) {
	if(c >='A' && c <='Z') return c - 'A';
	if(c >='a' && c <='z') return c - 71;
	if(c >='0' && c <='9') return c + 4;
	if(c == '+') return 62;
	if(c == '/') return 63;
	return -1;
}


//uint8_t getNumOfPadChars(unsigned int value, uint8_t padLength)
//{
	//uint8_t padChars = 0;
	//
	//if (value <10)
		//padChars = max(0, padLength-1);
	//else if (value < 100)
		//padChars = max(0, padLength-2);
	//else if (value < 1000)
		//padChars = max(0, padLength-3);
	//else if (value < 10000)
		//padChars = max(0, padLength-4);
	//else
		//padChars = max(0, padLength-5);
//
	//return padChars;	
//}
//
//
//byte writeCharWithPad(char* buffer, const char value, byte padLength)
//{
	//byte padChars = max(0, padLength-1);
	//
	////First pad
	//for(byte i=0;i<padChars;i++)
		//*(buffer++) = '0';
	//
	//*(buffer++) = value;
//
	//return padChars + 1;
//}
//
//
//
//byte writeCharArrWithPad(char* buffer, const char* value, byte padLength)
//{
	//byte valLen = strlen(value);
	//byte padChars = max(0, padLength-valLen);
	//
	////First pad
	//for(byte i=0;i<padChars;i++)
		//*(buffer++) = '0';
	//
	//for(byte i=0;i<valLen;i++)
		//*(buffer++) = *(value+i);
//
	//return padChars + valLen;
//}
//
////byte writeStrWithPad(char* buffer, String str, byte padLength)
////{
	////const char* strRaw = str.c_str();
	////
	////volatile const char* test = strRaw;
	////
	////return writeCharArrWithPad(buffer, strRaw, padLength);
////}
//
//byte writeByteWithPad(char* buffer, byte value, byte padLength)
//{
	//byte padChars =getNumOfPadChars(value, padLength);
//
	//for(byte i=0;i<padChars;i++)
		//*(buffer++) = '0';
	//
	//utoa(value, buffer, 10);
//
	//byte offset = 0;
	//while(*(buffer+offset)!= 0){offset++;} //However many characters it took
	//
	//return padChars + offset;	
//}
//
//byte writeWithPad(char* buffer, unsigned int value, byte padLength)
//{
	//byte padChars =getNumOfPadChars(value, padLength);
//
	//for(byte i=0;i<padChars;i++)
		//*(buffer++) = '0';
	//
	//utoa(value, buffer, 10);
//
	//byte offset = 0;
	//while(*(buffer+offset)!= 0){offset++;} //However many characters it took
	//
	//return padChars + offset;
//}
//
////byte writeCharArrToStrWithPad(String str, uint8_t idx, const char* value, byte padLength)
////{
////}
//
////byte writeStrToStrWithPad(String str, uint8_t idx, String value, uint8_t padLength)
////{
	//////Promote to signed before subtraction to detect negative
	////int8_t buffLength = max(0, (int8_t)padLength - (int8_t)value.length());
////
	////for(uint8_t i=0;i<buffLength;i++)
		////str[idx++] = '0';
////
////volatile char c1 = value[0];
////volatile char c2 = value[1];
////
	////for(uint8_t i=0;i<value.length();i++)
	////{
		////volatile char cc = value[i];
		////str[idx++] = value[i];
	////}
////}
//
////byte writeToStrWithPad(String str, uint8_t idx, unsigned int value, byte padLength)
////{
	////byte padChars = getNumOfPadChars(value, padLength);
	////
	////for(uint8_t i=0;i<padChars;i++)
		////str.setCharAt(idx++, '0');
		////
	//////Assume max number of digits is 10
	////char buffer[10];
	////utoa(value, buffer, 10);
	////
	////uint8_t offset=0;
	////while(*(buffer+offset)!= 0){str.setCharAt(idx++,buffer[offset]);} //However many characters it took
	////
	////return padChars + offset;
////}

