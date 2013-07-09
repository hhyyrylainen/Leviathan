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

//float ResolutionScaling::YScaleFactor = 1.f;
//
//float ResolutionScaling::XScaleFactor = 1.f;

int ResolutionScaling::Width = 0;

int ResolutionScaling::Height = 0;
// ------------------------------------ //

//float ResolutionScaling::GetXScaleFactor(){
//	return XScaleFactor;
//}
//float ResolutionScaling::GetYScaleFactor(){
//	return YScaleFactor;
//}
// ------------------------------------ //

void ResolutionScaling::SetResolution(int width, int height){
	Width = width;
	Height = height;

	//CalculateFactors();
}
//void ResolutionScaling::CalculateFactors(){
//	// use common math functions to get greatest common divisor //
//	int GCD = MMath::GreatestCommonDivisor(Height, Width);
//
//	// smallest possible values //
//	int DividedWidth = Width/GCD;
//	int DividedHeight = Height/GCD;
//
//	XScaleFactor = DividedWidth*1000.f;
//	YScaleFactor = DividedHeight*1000.f;
//}
// ------------------------------------ //
//float ResolutionScaling::ScalePromilleToFactorX(int x){
//	// from value range of 0-1000 to range of 0-XScaleFactor //
//	float percent = x/1000.f;
//	return percent*XScaleFactor;
//}

//float ResolutionScaling::ScalePromilleToFactorY(int y){
//	// from value range of 0-1000 to range of 0-YScaleFactor //
//	float percent = y/1000.f;
//	return percent*YScaleFactor;
//}


// ------------------------------------ //
//float ResolutionScaling::GetPromilleFactor(){
//	return 1000.f;
//}

//float ResolutionScaling::ScaleAbsoluteXToPromille(int x){
//	// get percentage of screen width and return that percentage of a promille //
//	return 1000.f*((float)x/DataStore::Get()->GetWidth());
//}
//
//float ResolutionScaling::ScaleAbsoluteYToPromille(int y){
//	// get percentage of screen height and return that percentage of a promille //
//	return 1000.f*((float)y/DataStore::Get()->GetHeight());
//}
//
//float Leviathan::ResolutionScaling::ScaleAbsoluteXToFactor(int x){
//	float returned = XScaleFactor*((float)x/DataStore::Get()->GetWidth())+0.5f;// rounding +0.5f //
//
//	return returned;
//}
//
//float Leviathan::ResolutionScaling::ScaleAbsoluteYToFactor(int y){
//	return YScaleFactor*((float)y/DataStore::Get()->GetHeight());
//}

float Leviathan::ResolutionScaling::ScaleTextSize(float size){
	// scale it so that it fits height perfectly //
	return size*(DataStore::Get()->GetHeight()/600.f);
}

DLLEXPORT float Leviathan::ResolutionScaling::UnScaleTextFromSize(float size){
	return size/(DataStore::Get()->GetHeight()/600.f);
}
