// ------------------------------------ //
#include "Event.h"

#include "Exceptions.h"
#include <boost/assign/list_of.hpp>
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::Event::Event(EVENT_TYPE type, BaseEventData* data) :
    Type(type), Data(data)
{
    // Check that types that require values have values //
    if(!Data) {
        // Check that the event has data //
        if(Type == EVENT_TYPE_FRAME_BEGIN || Type == EVENT_TYPE_TICK) {

            throw InvalidArgument("Event that requires data, didn't get it");
        }
    }
}

Event::~Event()
{
    // Delete our data //
    SAFE_DELETE(Data);
}
// ------------------------------------ //
DLLEXPORT EVENT_TYPE Leviathan::Event::GetType() const
{
    return Type;
}
// ------------------------------------ //
#ifdef LEVIATHAN_USING_SFML
DLLEXPORT void Leviathan::Event::AddDataToPacket(sf::Packet& packet) const
{
    // Add the type first //
    packet << (int)Type;

    // Add the data object //
    if(Data)
        Data->AddDataToPacket(packet);
}

DLLEXPORT Leviathan::Event::Event(sf::Packet& packet)
{
    // Get the type from the packet //
    int tmptype;

    if(!(packet >> tmptype)) {

        throw InvalidArgument("packet has invalid format");
    }

    // Set our type //
    Type = static_cast<EVENT_TYPE>(tmptype);

    // Load based on type //
    switch(Type) {
    case EVENT_TYPE_FRAME_BEGIN:
    case EVENT_TYPE_TICK: {
        Data = new FloatEventData(packet);
        break;
    }
    default:
        // No data required //
        Data = NULL;
    }
}
#endif // LEVIATHAN_USING_SFML
// ------------------------------------ //
DLLEXPORT FloatEventData* Leviathan::Event::GetFloatDataForEvent() const
{
    if(Type == EVENT_TYPE_TICK || Type == EVENT_TYPE_FRAME_BEGIN)
        return static_cast<FloatEventData*>(Data);
    return NULL;
}
// ------------------ GenericEvent ------------------ //
DLLEXPORT Leviathan::GenericEvent::GenericEvent(
    const std::string& type, const NamedVars& copyvals) :
    TypeStr(new std::string(type)),
    Variables(new NamedVars(copyvals))
{}

DLLEXPORT Leviathan::GenericEvent::GenericEvent(
    std::string* takeownershipstr, NamedVars* takeownershipvars) :
    TypeStr(takeownershipstr),
    Variables(takeownershipvars)
{}

DLLEXPORT Leviathan::GenericEvent::GenericEvent(const std::string& type) :
    TypeStr(new std::string(type)), Variables(new NamedVars())
{}

DLLEXPORT Leviathan::GenericEvent::~GenericEvent()
{
    // release memory //
    SAFE_DELETE(TypeStr);
    SAFE_RELEASE(Variables);
}
// ------------------------------------ //
#ifdef LEVIATHAN_USING_SFML
DLLEXPORT Leviathan::GenericEvent::GenericEvent(sf::Packet& packet)
{
    // Load data from the packet //
    std::unique_ptr<std::string> tmpstr(new std::string());
    if(!(packet >> *tmpstr)) {

        throw InvalidArgument("packet has invalid format");
    }

    // Try to get the named variables //
    std::unique_ptr<NamedVars> tmpvars(new NamedVars(packet));

    // Take the string away from the smart pointer //
    TypeStr = tmpstr.release();

    // Take the variables away //
    Variables = tmpvars.release();
}

DLLEXPORT void Leviathan::GenericEvent::AddDataToPacket(sf::Packet& packet) const
{
    // Add data to the packet //
    packet << *TypeStr;

    Variables->AddDataToPacket(packet);
}
#endif // LEVIATHAN_USING_SFML
// ------------------------------------ //
DLLEXPORT std::string* Leviathan::GenericEvent::GetTypePtr()
{
    return TypeStr;
}

DLLEXPORT std::string Leviathan::GenericEvent::GetType() const
{
    return *TypeStr;
}

DLLEXPORT const NamedVars Leviathan::GenericEvent::GetVariablesConst() const
{
    return *Variables;
}

DLLEXPORT NamedVars* Leviathan::GenericEvent::GetVariables()
{
    return Variables;
}
// ------------------------------------ //
DLLEXPORT NamedVars* Leviathan::GenericEvent::GetNamedVarsRefCounted()
{
    Variables->AddRef();
    return Variables;
}
// ------------------ FloatEventData ------------------ //
#ifdef LEVIATHAN_USING_SFML
DLLEXPORT Leviathan::FloatEventData::FloatEventData(sf::Packet& packet)
{

    packet >> FloatDataValue;

    if(!packet)
        throw InvalidArgument("packet has invalid format");
}

void Leviathan::FloatEventData::AddDataToPacket(sf::Packet& packet)
{
    packet << FloatDataValue;
}
#endif // LEVIATHAN_USING_SFML

DLLEXPORT Leviathan::FloatEventData::FloatEventData(float elapsed) : FloatDataValue(elapsed) {}
// ------------------ BaseEventData ------------------ //
BaseEventData::~BaseEventData() {}
