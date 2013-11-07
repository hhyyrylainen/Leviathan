#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_BASE_POSITIONABLE
#include "BasePositionable.h"
#endif
#include "BaseObject.h"
using namespace Leviathan;
// ------------------------------------ //
BasePositionable::BasePositionable() : QuatRotation(Float4::IdentityQuaternion()), Position(Float3(0)){

}

DLLEXPORT Leviathan::BasePositionable::BasePositionable(const Float3 &pos, const Float4 &orientation) : Position(pos), QuatRotation(orientation){

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
bool Leviathan::BasePositionable::BasePositionableCustomMessage(int message, void* data){
	switch(message){
	case ENTITYCUSTOMMESSAGETYPE_CHANGEWORLDPOSITION: {Position = *reinterpret_cast<Float3*>(data); PosUpdated(); return true;}


	}
	return false;
}

bool Leviathan::BasePositionable::BasePositionableCustomGetData(ObjectDataRequest* data){

	switch(data->RequestObjectPart){
	case ENTITYDATA_REQUESTTYPE_WORLDPOSITION: data->RequestResult = &Position; return true;

	}

	return false;
}
// ------------------------------------ //