#include "RadarModule.h"
#include "time.h"
#include <string.h>



RadarModule::RadarModule(void)
{
}

RadarModule::~RadarModule(void)
{
}

void RadarModule::serialConnect(void)
{
	handle = serialOpen("/dev/ttyUSB0", 115200);
	tcgetattr(handle, &options);     
	options.c_cflag |= CS8;
	options.c_cflag |= PARENB;
	options.c_ispeed = B115200;
	tcsetattr(handle, TCSANOW, &options);
}

int RadarModule::sendINIT(void)
{
	char* payload;
	int buff[] = { 73, 78, 73, 84, 4, 0, 0, 0, 0, 0, 0, 0 };
	
	char charArray[12];
	for (int x = 0; x < 12; x++) 
	{
		charArray[x] = (char)buff[x];
	}
	
	payload = charArray;
	

	write(handle, payload, 12);

	
	int Response = getResponse();
	if (Response != 0) {
		std::cout << "INIT Response Not Okay. Response: " << Response << "\n" ;
		return 1;
	}
	else {
		std::cout << "INIT Response Okay\n";
		return 0;
	}
}


int RadarModule::changeRange(int Range)
{

	char* payload;
	int buff[] = { 82, 82, 65, 73, 4, 0, 0, 0, Range, 0, 0, 0 };
	
	int S = sizeof(buff);
	
	char charArray[12];
	for (int x = 0; x < 12; x++) 
	{
		charArray[x] = (char)buff[x];
	}
	
	payload = charArray;
	

	write(handle, payload, 12);
	int Response = getResponse();
	if (Response != 0) {
		std::cout << "RRAI Response Not Okay. Response: " << Response << "\n" ;
		return 1;
	}
	else {
		std::cout << "RRAI Response Okay\n";
		return 0;
	}
	
}


int RadarModule::changeAngleMax(int8_t maxAngle)
{
	
	char* payload;
	
	//Initialize Char Buffer with Header
	char charbuff[12] = "MAAN";
	
	//Add Payload Size
	int PayloadSize = 4;
	charbuff[4] = char(PayloadSize);
	
	
	//Convert minAngle into 4 byte char array msg
	
	int32_t x = maxAngle;
	char *ptr = (char*)&x;
	char *msg = new char[5];
	for (int i = 0; i < 4; ++i, ++ptr)
		msg[i] = *ptr;
	
	//Place msg into charbuff
	for(int i = 0 ; i < 4 ; i++)
	{
		charbuff[i + 8] = msg[i];
	}
	
	
	//Send charbuff
	payload = charbuff;
	write(handle, payload, 12);
	int Response = getResponse();
	if (Response != 0) {
		std::cout << "MAAN Response Not Okay. Response: " << Response << "\n";
		return 1;
	}
	else {
		std::cout << "MAAN Response Okay\n";
		return 0;
	}
	
}

int RadarModule::changeAngleMin(int8_t minAngle)
{
	char* payload;
	
	//Initialize Char Buffer with Header
	char charbuff[12] = "MIAN";
	
	//Add Payload Size
	int PayloadSize = 4;
	charbuff[4] = char(PayloadSize);
	
	//Convert minAngle into 4 byte char array msg
	int32_t x = minAngle;
	char *ptr = (char*)&x;
	char *msg = new char[5];
	for (int i = 0; i < 4; ++i, ++ptr)
		msg[i] = *ptr;
	
	//Place msg into charbuff
	for (int i = 0; i < 4; i++)
	{
		charbuff[i + 8] = msg[i];
	}

	
	
	//Send charbuff
	payload = charbuff;
	write(handle, payload, 12);
	int Response = getResponse();
	if (Response != 0) {
		std::cout << "MIAN Response Not Okay. Response: " << Response << "\n";
		return 1;
	}
	else {
		std::cout << "MIAN Response Okay\n";
		return 0;
	}
	
}

int RadarModule::getResponse(void)
{
	int DataAvailable = 0;
	while (serialDataAvail(handle) <= 0)
	{
		//Do nothing
	}
	DataAvailable = 9;
	
	
	int ReturnedData[DataAvailable];
	for (int n = 0; n < DataAvailable; n++) 
	{
		ReturnedData[n] = serialGetchar(handle);
	}
	
	char HeaderRec[4] = { 0, 0, 0, 0 };
	for (int n = 0; n < 4; n++) {
		HeaderRec[n] = (char)ReturnedData[n];
	}
	
	int payloadSize[4] = { 0, 0, 0, 0 };
	for (int n = 4; n < 8; n++)
	{
		payloadSize[n - 4] = ReturnedData[n];
	}
	
	int payload = ReturnedData[8];

	serialFlush(handle);
	
	return payload;	
}

int RadarModule::getRange(void)
{
	char* payload;
	
	//Initialize Char Buffer with Header
	char charbuff[12] = "GNFD";
	//Add Payload Size
	int PayloadSize = 4;
	charbuff[4] = char(PayloadSize);
	
	//Add Payload
	int PDAT = 4;
	
	//TDAT is more accurate
	int TDAT = 8;
	
	
	charbuff[8] = char(TDAT);
	
	
	payload = charbuff;
	write(handle, payload, 12);
	int Response = getResponse();
	if (Response != 0) {
		std::cout << "GNFD Response Not Okay. Response: " << Response << "\n";
		serialFlush(handle);
		return 1;
	}
	else {
		int PDATReturn = getTDAT();
			
		return 0;
	}

}


int RadarModule::getPDAT(void){
	
	// Wait For PDAT
	int DataAvailable = 0;
	while (serialDataAvail(handle) <= 0)
	{
		//Do nothing
	}
	DataAvailable = serialDataAvail(handle);
		
	
	char Header[4];
	char* HeaderPointer = Header;
	u_int32_t PayloadLength[4] = { 0, 0, 0, 0 };
	u_int32_t* PayloadLengthPointer = PayloadLength ;
	u_int32_t FrameLength = 8;
	
	read(handle, HeaderPointer, 4);
	read(handle, PayloadLengthPointer, 4);
	
	
	if ((PayloadLength[0] < 96) && (PayloadLength[0] > 0) ) {
		int FramesCaptured = (PayloadLength[0] / FrameLength);
	
		// ADD 1 to allow for vector placement 
		int FramesSize = FramesCaptured + 1;
		
	
		uint16_t Distance[FramesSize], Magnitude[FramesSize];
		int16_t Speed[FramesSize], Angle[FramesSize];
	
		uint16_t* DistancePointer = Distance;
		uint16_t* MagPointer = Magnitude;
		int16_t* SpeedPointer = Speed;
		int16_t* AnglePointer = Angle;
	
		for (int n = 0; n < FramesCaptured; n++)
		{
			read(handle, DistancePointer, 2);
			read(handle, SpeedPointer, 2);
			read(handle, AnglePointer, 2);
			read(handle, MagPointer, 2);
		
			Distance[n + 1] = *DistancePointer;
			Speed[n + 1] = *SpeedPointer;
			Angle[n + 1] = *AnglePointer;
			Magnitude[n + 1] = *MagPointer;
		}
	
	
		serialFlush(handle);
	
		
		
		float DistanceInches = float(Distance[1]) * 0.4;
		float DistanceFeet = DistanceInches / 12;
	
		
		if (abs(Angle[1] / 100) <= MaxMinAngle) {	
			std::cout << "Radar distance: " << DistanceFeet << " Angle: " << Angle[1] << "\n";		
		}
	
	}
	
	return 0;
}



int RadarModule::getTDAT(void) {
	
	// Wait For PDAT
	int DataAvailable = 0;
	while (serialDataAvail(handle) <= 0)
	{
		//Do nothing
	}
	DataAvailable = serialDataAvail(handle);
		
	
	char Header[4];
	char* HeaderPointer = Header;
	u_int32_t PayloadLength[4] = { 0, 0, 0, 0 };
	u_int32_t* PayloadLengthPointer = PayloadLength;
	u_int32_t FrameLength = 8;
	
	read(handle, HeaderPointer, 4);
	read(handle, PayloadLengthPointer, 4);
	
	
	if ((PayloadLength[0] < 96) && (PayloadLength[0] > 0)) {
		int FramesCaptured = (PayloadLength[0] / FrameLength);
	
		// ADD 1 to allow for vector placement 
		int FramesSize = FramesCaptured + 1;
		
	
		uint16_t Distance[FramesSize], Magnitude[FramesSize];
		int16_t Speed[FramesSize], Angle[FramesSize];
	
		uint16_t* DistancePointer = Distance;
		uint16_t* MagPointer = Magnitude;
		int16_t* SpeedPointer = Speed;
		int16_t* AnglePointer = Angle;
	
		for (int n = 0; n < FramesCaptured; n++)
		{
			read(handle, DistancePointer, 2);
			read(handle, SpeedPointer, 2);
			read(handle, AnglePointer, 2);
			read(handle, MagPointer, 2);
		
			Distance[n + 1] = *DistancePointer;
			Speed[n + 1] = *SpeedPointer;
			Angle[n + 1] = *AnglePointer;
			Magnitude[n + 1] = *MagPointer;
		}
	
	
		serialFlush(handle);
	
		
		
		float DistanceInches = float(Distance[1]) * 0.4;
		float DistanceFeet = DistanceInches / 12;
	
		
		if (abs(Angle[1] / 100) <= MaxMinAngle) {	
			std::cout << "Radar distance: " << DistanceFeet << " Angle: " << (Angle[1]/100) << "\n";		
		}
	
	}
	
	return 0;
}

