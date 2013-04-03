#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SMOOTHEDPOSITION
#include "SmoothedPosition.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
SmoothedPosition::SmoothedPosition(){
	s_X = 0;
	s_Y = 0;
	s_Z = 0;
	s_Pitch = 0;
	s_Yaw = 0;
	s_Roll = 0;
	SmoothUpdated = true;
}
SmoothedPosition::~SmoothedPosition(){

}
// ------------------------------------ //
void SmoothedPosition::SmoothValues(){
	int difference = 0;
	// smooth values towards real values for smooth movement //
	// check is jump too high - directly change the value then //
	if(s_X != X){
		difference = X-s_X;
		// smooth difference //
		s_X += difference/OBJECT_SMOOTH; 

		SmoothUpdated = true;
		// this needed to be done before forcing the value to positive //

		FORCE_POSITIVE(difference);
		if(difference > 300 * UNIT_SCALE){
			s_X = X;
		}
	}
	if(s_Y != Y){
		difference = Y-s_Y;
		// smooth difference //
		s_Y += difference/OBJECT_SMOOTH; 

		SmoothUpdated = true;
		// this needed to be done before forcing the value to positive //

		FORCE_POSITIVE(difference);
		if(difference > 300 * UNIT_SCALE){
			s_Y = Y;
		}
	}
	if(s_Z != Z){
		difference = Z-s_Z;
		// smooth difference //
		s_Z += difference/OBJECT_SMOOTH; 

		SmoothUpdated = true;
		// this needed to be done before forcing the value to positive //

		FORCE_POSITIVE(difference);
		if(difference > 300 * UNIT_SCALE){
			s_Z = Z;
		}
	}

	// smooth pitch yaw and roll //
	if(s_Pitch != Pitch){
		difference = Pitch-s_Pitch;
		// smooth difference //
		s_Pitch += difference/OBJECT_SMOOTH; 

		SmoothUpdated = true;
		// this needed to be done before forcing the value to positive //

		FORCE_POSITIVE(difference);
		if(difference > 300 * UNIT_SCALE){
			s_Pitch = Pitch;
		}
	}
	if(s_Yaw != Yaw){
		difference = Yaw-s_Yaw;
		// smooth difference //
		s_Yaw += difference/OBJECT_SMOOTH; 

		SmoothUpdated = true;
		// this needed to be done before forcing the value to positive //

		FORCE_POSITIVE(difference);
		if(difference > 300 * UNIT_SCALE){
			s_Yaw = Yaw;
		}
	}
	if(s_Roll != Roll){
		difference = Roll-s_Roll;
		// smooth difference //
		s_Roll += difference/OBJECT_SMOOTH; 

		SmoothUpdated = true;
		// this needed to be done before forcing the value to positive //

		FORCE_POSITIVE(difference);
		if(difference > 300 * UNIT_SCALE){
			s_Roll = Roll;
		}
	}

}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //