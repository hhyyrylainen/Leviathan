// ------------------------------------ //
#ifndef LEVIATHAN_BASE_POSITIONABLE
#include "BasePositionable.h"
#endif
#include "BaseObject.h"
#include "Exceptions.h"
using namespace Leviathan;
// ------------------------------------ //
BasePositionable::BasePositionable() : QuatRotation(Float4::IdentityQuaternion()), Position(Float3(0)){

}

DLLEXPORT Leviathan::BasePositionable::BasePositionable(const Float3 &pos, const Float4 &orientation) :
    Position(pos), QuatRotation(orientation)
{

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
Float4 Leviathan::BasePositionable::GetOrientation() const{
	return QuatRotation;
}

DLLEXPORT Float4 Leviathan::BasePositionData::GetRotation() const{
    return QuatRotation;
}

void Leviathan::BasePositionable::GetOrientation(Float4 &receiver) const{
	receiver = QuatRotation;
}

DLLEXPORT void Leviathan::BasePositionable::GetRotation(Float4 &receiver) const{
	receiver = QuatRotation;
}
// ------------------------------------ //
void BasePositionable::GetPosElements(float &outx, float &outy, float &outz){
	outx = Position.X;
	outy = Position.Y;
	outz = Position.Z;
}

DLLEXPORT Float3 Leviathan::BasePositionable::GetPos() const{
	return Position;
}

DLLEXPORT void Leviathan::BasePositionable::GetPos(Float3 &receiver) const{
    receiver = Position;
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

DLLEXPORT void Leviathan::BasePositionable::SetPosComponents(const float &x, const float &y, const float &z){
	Position.X = x;
	Position.Y = y;
	Position.Z = z;
	PosUpdated();
}

DLLEXPORT void Leviathan::BasePositionable::SetPos(const Float3 &pos){
	Position = pos;
	PosUpdated();
}

DLLEXPORT void Leviathan::BasePositionable::SetPosition(const Float3 &pos){
	Position = pos;
	PosUpdated();
}

DLLEXPORT void Leviathan::BasePositionable::SetOrientation(const Float4 &quat){
	QuatRotation = quat;
	OrientationUpdated();
}

DLLEXPORT void Leviathan::BasePositionable::SetOrientationComponents(const float &x, const float &y,
    const float &z, const float &w)
{
    QuatRotation = Float4(x, y, z, w);
    OrientationUpdated();
}

void Leviathan::BasePositionable::PosUpdated(){

}

void Leviathan::BasePositionable::OrientationUpdated(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::BasePositionable::ApplyPositionDataObject(const BasePositionData &pos){

    Position = pos.Position;
    PosUpdated();

    QuatRotation = pos.QuatRotation;
    OrientationUpdated();
}
// ------------------------------------ //
bool Leviathan::BasePositionable::BasePositionableCustomMessage(int message, void* data){
	switch(message){
        case ENTITYCUSTOMMESSAGETYPE_CHANGEWORLDPOSITION:
        {
            Position = *reinterpret_cast<Float3*>(data); PosUpdated(); return true;
        }


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
DLLEXPORT void Leviathan::BasePositionable::AddPositionAndRotationToPacket(sf::Packet &packet){

    packet << Position;
    packet << QuatRotation;
}

DLLEXPORT void Leviathan::BasePositionable::ApplyPositionAndRotationFromPacket(sf::Packet &packet){

    // First get the data //
    Float4 quaternion;
    Float3 pos;
    
    packet >> pos;
    packet >> quaternion;
    
    // Don't apply if any of the reads have failed //
    if(!packet){
        // Some read has failed //
        throw InvalidArgument("invalid packet");
    }

    // Apply the data //
    SetPos(pos);
    SetOrientation(quaternion);
}

DLLEXPORT bool Leviathan::BasePositionable::LoadPositionFromPacketToHolder(sf::Packet &packet,
    BasePositionData &target)
{
    packet >> target.Position;
    packet >> target.QuatRotation;
    
    return packet ? true: false;
}
