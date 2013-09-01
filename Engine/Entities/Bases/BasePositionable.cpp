#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_POSITIONABLE
#include "BasePositionable.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
BasePositionable::BasePositionable(){
	X = 0;
	Y = 0;
	Z = 0;
	Pitch = 0;
	Yaw = 0;
	Roll = 0;
}
BasePositionable::~BasePositionable(){

}
// ------------------------------------ //
void BasePositionable::SetPosX(const float &x){
	X = x;
	PosUpdated();
}
void BasePositionable::SetPosY(const float &y){
	Y = y;
	PosUpdated();
}
void BasePositionable::SetPosZ(const float &z){
	Z = z;
	PosUpdated();
}
// ------------------------------------ //
void BasePositionable::SetPitch(int pitch){
	Pitch = pitch;
	OrientationUpdated();
}
void BasePositionable::SetYaw(int yaw){
	Yaw = yaw;
	OrientationUpdated();
}
void BasePositionable::SetRoll(int roll){
	Roll = roll;
	OrientationUpdated();
}
// ------------------------------------ //
void BasePositionable::GetOrientation(int &outpitch, int &outyaw, int &outroll){
	outpitch = Pitch;
	outyaw = Yaw;
	outroll = Roll;
}
int BasePositionable::GetPitch(){
	return Pitch;
}
int BasePositionable::GetYaw(){
	return Yaw;
}
int BasePositionable::GetRoll(){
	return Roll;
}


// ------------------------------------ //
void BasePositionable::GetPos(float &outx, float &outy, float &outz){
	outx = X;
	outy = Y;
	outz = Z;
}
float BasePositionable::GetXPos(){
	return X;
}
float BasePositionable::GetYPos(){
	return Y;
}
float BasePositionable::GetZPos(){
	return Z;
}

DLLEXPORT void Leviathan::BasePositionable::SetPos(const float &x, const float &y, const float &z){
	X = x;
	Y = y;
	Z = z;
	PosUpdated();
}

DLLEXPORT void Leviathan::BasePositionable::SetOrientation(int pitch, int yaw, int roll){
	Pitch = pitch;
	Yaw = yaw;
	Roll = roll;
}

void Leviathan::BasePositionable::PosUpdated(){

}

void Leviathan::BasePositionable::OrientationUpdated(){

}

// ------------------------------------ //