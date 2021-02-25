#include "RadarModule.h"

using namespace std;


int main(int argc, char *argv[])
{
	int Range = 0;
	int MaxMin = 2;
	bool run = true;
	
	
	// Configure Radar
	RadarModule Radar;
	Radar.serialConnect();
	Radar.sendINIT();
	Radar.changeRange(Range);
	Radar.setMaxMinAngle(MaxMin);
	Radar.getRange();
	
	
	while (run)
	{
		
		// Get range 
		Radar.getRange();
	
	}
	


	return 0;
}