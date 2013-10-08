#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_POSITIONABLE
#include "BasePositionable.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
BasePositionable::BasePositionable() : QuatRotation(Float4::IdentityQuaternion()), Position(Float3(0)){

}
BasePositionable::~BasePositionable(){

}
// ------------------------------------ //
void BasePositionable::SetPosX(const float &x){
	Position.X = x;
	PosUpdated();
}
void BasePositionable::SetPosY(const float &y){
	Position.Y = y;
	PosUpdated();
}
void BasePositionable::SetPosZ(const float &z){
	Position.Z = z;
	PosUpdated();
}
// ------------------------------------ //
Float4 BasePositionable::GetOrientation(){
	return QuatRotation;
}
// ------------------------------------ //
void BasePositionable::GetPos(float &outx, float &outy, float &outz){
	outx = Position.X;
	outy = Position.Y;
	outz = Position.Z;
}

DLLEXPORT Float3 Leviathan::BasePositionable::GetPos(){
	return Position;
}


float BasePositionable::GetXPos(){
	return Position.X;
}
float BasePositionable::GetYPos(){
	return Position.Y;
}
float BasePositionable::GetZPos(){
	return Position.Z;
}

DLLEXPORT void Leviathan::BasePositionable::SetPos(const float &x, const float &y, const float &z){
	Position.X = x;
	Position.Y = y;
	Position.Z = z;
	PosUpdated();
}

DLLEXPORT void Leviathan::BasePositionable::SetPos(const Float3 &pos){
	Position = pos;
	PosUpdated();
}

DLLEXPORT void Leviathan::BasePositionable::SetOrientation(const Float4 &quat){
	QuatRotation = quat;
	OrientationUpdated();
}

void Leviathan::BasePositionable::PosUpdated(){

}

void Leviathan::BasePositionable::OrientationUpdated(){

}

// ------------------------------------ //