#pragma once
#include <wiringSerial.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>			//Used for UART
#include <fcntl.h>			//Used for UART
#include <iostream>
#include <array>
#include <stdlib.h>
#include <math.h>

class RadarModule
{
	
	int handle;
	
	struct termios options ;
	
	
	int MaxMinAngle;
	/* getResponse: Returns command response aknowledge message. Acknowledge information:
	0 = OK, 1 = Unknown command, 2 = Invalid parameter
	value, 3 = Invalid RPST version, 4 = Uart error(parity,framing,
	noise), 5 = Sensor busy*/
	int getResponse(void);
	
	int getPDAT(void);
	
	int getTDAT(void);
	
public:
	
	
	RadarModule(void);
	~RadarModule(void);
	
	/*Sets Max Min angle detection*/
	void setMaxMinAngle(int MAXMININ)
	{
		MaxMinAngle = MAXMININ;
	}
	
	/*Initiallized Serial Port and sets configs.*/
	void serialConnect(void);
	
	/*Sends INIT command. Returns 0 if okay else 1.*/
	int sendINIT(void);
	
	/* Max range 0=5m, 1=10m, 2=30m, 3=100m
	Returns 0 for range changed succesfully else 0.*/	
	int changeRange(int Range);
	
	/*Min angle between -90 to 90 degrees*/
	int changeAngleMin(int8_t MinAngle);
	
	/*Max angle between -90 to 90 degrees. 
	 Returns 0 for range changed succesfully else 0.*/
	int changeAngleMax(int8_t MaxAngle);
	
	
	int getRange(void);
	
	
};

