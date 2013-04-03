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
void BasePositionable::SetPosX(int x){
	X = x;
	PosUpdated();
}
void BasePositionable::SetPosY(int y){
	Y = y;
	PosUpdated();
}
void BasePositionable::SetPosZ(int z){
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
void BasePositionable::GetPos(int &outx, int &outy, int &outz){
	outx = X;
	outy = Y;
	outz = Z;
}
int BasePositionable::GetXPos(){
	return X;
}
int BasePositionable::GetYPos(){
	return Y;
}
int BasePositionable::GetZPos(){
	return Z;
}
// ------------------------------------ //