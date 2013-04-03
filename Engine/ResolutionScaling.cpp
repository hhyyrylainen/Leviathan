#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RESOLUTIONSCALING
#include "ResolutionScaling.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "DataStore.h"

ResolutionScaling::ResolutionScaling(){

}
ResolutionScaling::~ResolutionScaling(){

}

float ResolutionScaling::YScaleFactor = 1000.f;

float ResolutionScaling::XScaleFactor = 1000.f;

int ResolutionScaling::Width = 0;

int ResolutionScaling::Height = 0;
// ------------------------------------ //

float ResolutionScaling::GetXScaleFactor(){
	return XScaleFactor;
}
float ResolutionScaling::GetYScaleFactor(){
	return YScaleFactor;
}
// ------------------------------------ //

void ResolutionScaling::SetResolution(int width, int height){
	Width = width;
	Height = height;

	CalculateFactors();
}
void ResolutionScaling::CalculateFactors(){
	// find out what is the greatest common divisor //
	int GCD = 0;
	//for(int i = 1; i <= Width; i++){
	//	if(Width % i == 0){
	//		// exact division check the other and if it's exact too then found new common divisor //
	//		if(Height % i == 0){
	//			GCD = i;
	//		}
	//	}
	//}
	//if(GCD == 0){
	//	// error //

	//}
	// use common math functions to get greatest common divisor //
	GCD = MMath::GreatestCommonDivisor(Height, Width);

	// smallest possible values //
	int DividedWidth = Width/GCD;
	int DividedHeight = Height/GCD;

	XScaleFactor = DividedWidth*1000.f;
	YScaleFactor = DividedHeight*1000.f;
}
// ------------------------------------ //
float ResolutionScaling::ScalePromilleToFactorX(int x){
	// from value range of 0-1000 to range of 0-XScaleFactor //
	float percent = x/1000.f;
	return percent*XScaleFactor;
}

float ResolutionScaling::ScalePromilleToFactorY(int y){
	// from value range of 0-1000 to range of 0-YScaleFactor //
	float percent = y/1000.f;
	return percent*YScaleFactor;
}


// ------------------------------------ //
float ResolutionScaling::GetPromilleFactor(){
	return 1000.f;
}

float ResolutionScaling::ScaleAbsoluteXToPromille(int x){
	// get percentage of screen width and return that percentage of a promille //
	return 1000.f*((float)x/DataStore::Get()->GetWidth());
}

float ResolutionScaling::ScaleAbsoluteYToPromille(int y){
	// get percentage of screen height and return that percentage of a promille //
	return 1000.f*((float)y/DataStore::Get()->GetHeight());
}

float Leviathan::ResolutionScaling::ScaleAbsoluteXToFactor(int x){
	float returned = XScaleFactor*((float)x/DataStore::Get()->GetWidth())+0.5f;// rounding +0.5f //

	return returned;
}

float Leviathan::ResolutionScaling::ScaleAbsoluteYToFactor(int y){
	return YScaleFactor*((float)y/DataStore::Get()->GetHeight());
}

float Leviathan::ResolutionScaling::ScaleTextSize(float size){
	// scale it so that it fits height perfectly //
	//return size*(YScaleFactor/DataStore::Get()->GetHeight());
	float returned = size*(DataStore::Get()->GetHeight()/600.f);

	return returned;
}
