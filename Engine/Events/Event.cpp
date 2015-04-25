// ------------------------------------ //
#include "Event.h"

#include <boost/assign/list_of.hpp>
#include "Exceptions.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Event::Event(EVENT_TYPE type, BaseEventData* data) : Type(type), Data(data){
	// Check that types that require values have values //
	if(!Data){
		// Check that the event has data //
        if(Type == EVENT_TYPE_PHYSICS_BEGIN || Type == EVENT_TYPE_SHOW ||
            Type == EVENT_TYPE_FRAME_BEGIN || Type == EVENT_TYPE_FRAME_END ||
			Type == EVENT_TYPE_TICK)
        {

            throw InvalidArgument("Event that requires data, didn't get it");
        }
	}
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

		throw InvalidArgument("packet has invalid format");
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
        case EVENT_TYPE_PHYSICS_RESIMULATE_SINGLE:
        {
            Data = new ResimulateSingleEventData(packet);
        }
        break;
        case EVENT_TYPE_FRAME_BEGIN:
        case EVENT_TYPE_FRAME_END:
        case EVENT_TYPE_TICK:
		{
			Data = new IntegerEventData(packet);
		}
		break;
        case EVENT_TYPE_CLIENT_INTERPOLATION:
        {
            Data = new ClientInterpolationEventData(packet);
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

DLLEXPORT ResimulateSingleEventData* Leviathan::Event::GetDataForResimulateSingleEvent() const{
	if(Type == EVENT_TYPE_PHYSICS_RESIMULATE_SINGLE)
		return static_cast<ResimulateSingleEventData*>(Data);
	return NULL;
}

DLLEXPORT ClientInterpolationEventData* Event::GetDataForClientInterpolationEvent() const{
	if(Type == EVENT_TYPE_CLIENT_INTERPOLATION)
		return static_cast<ClientInterpolationEventData*>(Data);
	return NULL;
}

DLLEXPORT IntegerEventData* Leviathan::Event::GetIntegerDataForEvent() const{
	if(Type == EVENT_TYPE_TICK || Type == EVENT_TYPE_FRAME_BEGIN || Type == EVENT_TYPE_FRAME_END)
		return static_cast<IntegerEventData*>(Data);
	return NULL;
}
// ------------------ GenericEvent ------------------ //
DLLEXPORT Leviathan::GenericEvent::GenericEvent(const std::string &type, const NamedVars &copyvals) :
    TypeStr(new std::string(type)), Variables(new NamedVars(copyvals))
{

}

DLLEXPORT Leviathan::GenericEvent::GenericEvent(std::string* takeownershipstr, NamedVars* takeownershipvars) :
    TypeStr(takeownershipstr), Variables(takeownershipvars)
{

}

DLLEXPORT Leviathan::GenericEvent::GenericEvent(const std::string &type) :
    TypeStr(new std::string(type)), Variables(new NamedVars())
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
	unique_ptr<std::string> tmpstr(new std::string());
	if(!(packet >> *tmpstr)){

		throw InvalidArgument("packet has invalid format");
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
DLLEXPORT std::string* Leviathan::GenericEvent::GetTypePtr(){
	return TypeStr;
}

DLLEXPORT std::string Leviathan::GenericEvent::GetType() const{
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
// ------------------ ClientInterpolationEventData ------------------ //
void ClientInterpolationEventData::CalculatePercentage(){

    Percentage = TimeInTick/(float)TICKSPEED;

    // Clamp the value to avoid breaking animations //
    if(Percentage < 0){
        
        Percentage = 0;
        
    } else if(Percentage > 1.f){
        
        Percentage = 1.f;
    }    
}

DLLEXPORT ClientInterpolationEventData::ClientInterpolationEventData(int tick, int mspassed) :
    TickNumber(tick), TimeInTick(mspassed)
{
    CalculatePercentage();
}

DLLEXPORT ClientInterpolationEventData::ClientInterpolationEventData(sf::Packet &packet){

    packet >> TickNumber >> TimeInTick;

    if(!packet)
        throw InvalidArgument("packet for ClientInterpolationEventData is invalid");

    CalculatePercentage();
}

DLLEXPORT void ClientInterpolationEventData::AddDataToPacket(sf::Packet &packet){

    packet << TickNumber << TimeInTick;
}
// ------------------ PhysicsStartEventData ------------------ //
void Leviathan::PhysicsStartEventData::AddDataToPacket(sf::Packet &packet){
	// Add our data //
	packet << TimeStep;
}

Leviathan::PhysicsStartEventData::PhysicsStartEventData(const float &time, void* worldptr) :
    TimeStep(time), GameWorldPtr(worldptr)
{

}

DLLEXPORT Leviathan::PhysicsStartEventData::PhysicsStartEventData(sf::Packet &packet){
	// Load our data //
	if(!(packet >> TimeStep)){

		throw InvalidArgument("packet has invalid format");
	}

	// This doesn't make any sense to be stored //
	GameWorldPtr = NULL;
}
// ------------------ ShowEventData ------------------ //
DLLEXPORT Leviathan::ShowEventData::ShowEventData(sf::Packet &packet){
	if(!(packet >> IsShown)){

		throw InvalidArgument("packet has invalid format");
	}
}

DLLEXPORT Leviathan::ShowEventData::ShowEventData(bool shown) : IsShown(shown){

}

void Leviathan::ShowEventData::AddDataToPacket(sf::Packet &packet){
	packet << IsShown;
}
// ------------------ IntegerEventData ------------------ //
DLLEXPORT Leviathan::IntegerEventData::IntegerEventData(sf::Packet &packet){

    packet >> IntegerDataValue;

    if(!packet)
        throw InvalidArgument("packet has invalid format");
}

DLLEXPORT Leviathan::IntegerEventData::IntegerEventData(int ticknumber) :
    IntegerDataValue(ticknumber)
{

}

void Leviathan::IntegerEventData::AddDataToPacket(sf::Packet &packet){
	packet << IntegerDataValue;
}
// ------------------ BaseEventData ------------------ //
BaseEventData::~BaseEventData(){
	
}

// ------------------ ResimulateSingleEventData ------------------ //
DLLEXPORT Leviathan::ResimulateSingleEventData::ResimulateSingleEventData(sf::Packet &packet) :
    GameWorldPtr(NULL), Target(NULL), TimeInPast(0)
{
#ifdef SFML_HAS_64_BIT_VALUES_PACKET
    
    packet >> TimeInPast;
    
#endif //SFML_HAS_64_BIT_VALUES_PACKET
    
    if(!packet)
        throw InvalidArgument("packet has invalid format");
}

DLLEXPORT Leviathan::ResimulateSingleEventData::ResimulateSingleEventData(int64_t resimulateremaining,
    BaseConstraintable* resimulated, void* worldptr) :
    TimeInPast(resimulateremaining), Target(resimulated), GameWorldPtr(worldptr)
{

}

void Leviathan::ResimulateSingleEventData::AddDataToPacket(sf::Packet &packet){

#ifdef SFML_HAS_64_BIT_VALUES_PACKET
    
    packet << TimeInPast;

#endif //SFML_HAS_64_BIT_VALUES_PACKET
}
