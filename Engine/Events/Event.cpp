#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_EVENT
#include "Event.h"
#endif
#include <boost/assign/list_of.hpp>
#include "Exceptions/ExceptionInvalidArgument.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Event::Event(EVENT_TYPE type, BaseEventData* data) : Type(type), Data(data){
	// Check that types that require values have values //
#ifdef _DEBUG
	if(!Data){
		// Check that the event has data //
		assert(Type != EVENT_TYPE_PHYSICS_BEGIN && Type != EVENT_TYPE_SHOW && Type != EVENT_TYPE_FRAME_BEGIN && Type != EVENT_TYPE_FRAME_END &&
			Type != EVENT_TYPE_TICK && "Event that requires data didn't get it");
	}

#endif // _DEBUG
}

Event::~Event(){
	// Delete our data //
	SAFE_DELETE(Data);
}
// ------------------------------------ //
DLLEXPORT EVENT_TYPE Leviathan::Event::GetType() const{
	return Type;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::Event::AddDataToPacket(sf::Packet &packet) const{
	// Add the type first //
	packet << (int)Type;

	// Add the data object //
	if(Data)
		Data->AddDataToPacket(packet);
}

DLLEXPORT Leviathan::Event::Event(sf::Packet &packet){
	// Get the type from the packet //
	int tmptype;

	if(!(packet >> tmptype)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"packet", L"");
	}

	// Set our type //
	Type = static_cast<EVENT_TYPE>(tmptype);

	// Load based on type //
	switch(Type){
	case EVENT_TYPE_PHYSICS_BEGIN:
		{
			// Load data //
			Data = new PhysicsStartEventData(packet);
		}
		break;
	case EVENT_TYPE_SHOW:
		{
			Data = new ShowEventData(packet);
		}
		break;
	case EVENT_TYPE_FRAME_BEGIN:
	case EVENT_TYPE_FRAME_END:
	case EVENT_TYPE_TICK:
		{
			Data = new IntegerEventData(packet);
		}
		break;
	default:
		// No data required //
		Data = NULL;
	}
}
// ------------------------------------ //
DLLEXPORT PhysicsStartEventData* Leviathan::Event::GetDataForPhysicsStartEvent() const{
	if(Type == EVENT_TYPE_PHYSICS_BEGIN)
		return static_cast<PhysicsStartEventData*>(Data);
	return NULL;
}

DLLEXPORT ShowEventData* Leviathan::Event::GetDataForShowEvent() const{
	if(Type == EVENT_TYPE_SHOW)
		return static_cast<ShowEventData*>(Data);
	return NULL;
}

DLLEXPORT IntegerEventData* Leviathan::Event::GetDataForTickEvent() const{
	if(Type == EVENT_TYPE_TICK || Type == EVENT_TYPE_FRAME_BEGIN || Type == EVENT_TYPE_FRAME_END)
		return static_cast<IntegerEventData*>(Data);
	return NULL;
}
// ------------------ GenericEvent ------------------ //
DLLEXPORT Leviathan::GenericEvent::GenericEvent(const wstring &type, const NamedVars &copyvals) : TypeStr(new wstring(type)), 
	Variables(new NamedVars(copyvals))
{

}

DLLEXPORT Leviathan::GenericEvent::GenericEvent(wstring* takeownershipstr, NamedVars* takeownershipvars) : TypeStr(takeownershipstr), 
	Variables(takeownershipvars)
{

}
DLLEXPORT Leviathan::GenericEvent::~GenericEvent(){
	// release memory //
	SAFE_DELETE(TypeStr);
	SAFE_RELEASE(Variables);
}
// ------------------------------------ //
DLLEXPORT Leviathan::GenericEvent::GenericEvent(sf::Packet &packet){
	// Load data from the packet //
	unique_ptr<wstring> tmpstr(new wstring());
	if(!(packet >> *tmpstr)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"packet", L"");
	}

	// Try to get the named variables //
	unique_ptr<NamedVars> tmpvars(new NamedVars(packet));

	// Take the string away from the smart pointer //
	TypeStr = tmpstr.release();

	// Take the variables away //
	Variables = tmpvars.release();
}

DLLEXPORT void Leviathan::GenericEvent::AddDataToPacket(sf::Packet &packet) const{
	// Add data to the packet //
	packet << *TypeStr;

	Variables->AddDataToPacket(packet);
}
// ------------------------------------ //
DLLEXPORT wstring* Leviathan::GenericEvent::GetTypePtr(){
	return TypeStr;
}

DLLEXPORT wstring Leviathan::GenericEvent::GetType() const{
	return *TypeStr;
}

DLLEXPORT const NamedVars Leviathan::GenericEvent::GetVariablesConst() const{
	return *Variables;
}

DLLEXPORT NamedVars* Leviathan::GenericEvent::GetVariables(){
	return Variables;
}
// ------------------------------------ //
DLLEXPORT NamedVars* Leviathan::GenericEvent::GetNamedVarsRefCounted(){
	Variables->AddRef();
	return Variables;
}
// ------------------ PhysicsStartEventData ------------------ //
void Leviathan::PhysicsStartEventData::AddDataToPacket(sf::Packet &packet){
	// Add our data //
	packet << TimeStep;
}

Leviathan::PhysicsStartEventData::PhysicsStartEventData(const float &time, void* worldptr) : TimeStep(time), GameWorldPtr(worldptr){

}

DLLEXPORT Leviathan::PhysicsStartEventData::PhysicsStartEventData(sf::Packet &packet){
	// Load our data //
	if(!(packet >> TimeStep)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"packet", L"");
	}

	// This doesn't make any sense to be stored //
	GameWorldPtr = NULL;
}
// ------------------ ShowEventData ------------------ //
DLLEXPORT Leviathan::ShowEventData::ShowEventData(sf::Packet &packet){
	if(!(packet >> IsShown)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"packet", L"");
	}
}

DLLEXPORT Leviathan::ShowEventData::ShowEventData(bool shown) : IsShown(shown){

}

void Leviathan::ShowEventData::AddDataToPacket(sf::Packet &packet){
	packet << IsShown;
}
// ------------------ IntegerEventData ------------------ //
DLLEXPORT Leviathan::IntegerEventData::IntegerEventData(sf::Packet &packet){
	if(!(packet >> IntegerDataValue)){

		throw ExceptionInvalidArgument(L"packet has invalid format", 0, __WFUNCTION__, L"packet", L"");
	}
}

DLLEXPORT Leviathan::IntegerEventData::IntegerEventData(int ticknumber) : IntegerDataValue(ticknumber){

}

void Leviathan::IntegerEventData::AddDataToPacket(sf::Packet &packet){
	packet << IntegerDataValue;
}
