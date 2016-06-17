// ------------------------------------ //
#include "GameSpecificPacketHandler.h"

#include "Exceptions.h"
#include "../Utility/Convert.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::GameSpecificPacketHandler::GameSpecificPacketHandler(NetworkInterface* usetoreport) :
    IsVectorSorted(false)
{
	Staticaccess = this;
}

DLLEXPORT Leviathan::GameSpecificPacketHandler::~GameSpecificPacketHandler(){
	Staticaccess = NULL;
}

DLLEXPORT GameSpecificPacketHandler* Leviathan::GameSpecificPacketHandler::Get(){
	return Staticaccess;
}

GameSpecificPacketHandler* Leviathan::GameSpecificPacketHandler::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT void Leviathan::GameSpecificPacketHandler::PassGameSpecificDataToPacket(GameSpecificPacketData*
    datatosend, sf::Packet &packet)
{
	// Try to find a handler for this //
	auto handlerobject = _FindFactoryForType(datatosend->TypeIDNumber, datatosend->IsRequest);

	// Now that there is a factory it's safe to start putting it together by adding the TypeID first //
	packet << datatosend->TypeIDNumber << datatosend->IsRequest;

	// Now the actual data //

	// Factory already knows what it does so let it do it //
	if(!handlerobject->SerializeToPacket(datatosend, packet)){

		throw InvalidArgument("invalid custom request packet for writing to packet");
	}

	
	// The packet should now be fine //
}

DLLEXPORT std::shared_ptr<GameSpecificPacketData> Leviathan::GameSpecificPacketHandler::ReadGameSpecificPacketFromPacket(
    bool responsepacket, sf::Packet &packet)
{
	// Get the basic data from the packet //
	int typeidnumber = -1;
	bool packetisrequest;

	if(!(packet >> typeidnumber)){

		throw InvalidAccess("invalid received custom packet base format");
	}

	if(!(packet >> packetisrequest)){

		throw InvalidAccess("invalid received custom packet base format");
	}

	if(packetisrequest != !responsepacket){
        
		// The packet might be corrupted or something //
		Logger::Get()->Warning("GameSpecificPacketHandler: packet has inconsistent request identifier, expected "+
            Convert::ToString(responsepacket)+" but packet was "+
            Convert::ToString(packetisrequest));
		return NULL;
	}

	// Try to find a handler for this //
	auto handlerobject = _FindFactoryForType(typeidnumber, packetisrequest);

	if(!handlerobject){

		Logger::Get()->Warning("GameSpecificPacketHandler: couldn't find a handler for a packet of type "+
            Convert::ToString(typeidnumber)+" and"
			" request state: "+Convert::ToString(packetisrequest));
		return NULL;
	}

	// Factory already knows what it does so let it do it //
	return handlerobject->UnSerializeObjectFromPacket(packet);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::GameSpecificPacketHandler::RegisterNewTypeFactory(BaseGameSpecificPacketFactory*
    newdfactoryobject)
{
	// Check does it exist already //
	if(_FindFactoryForType(newdfactoryobject->TypeIDNumber, newdfactoryobject->HandlesRequests)){

		assert(0 && "Trying to register a type that is already registered for custom packages");
	}

	// Add //
	AllPacketFactories.push_back(shared_ptr<BaseGameSpecificPacketFactory>(newdfactoryobject));

	// Mark as unsorted //
	IsVectorSorted = false;
}
// ------------------------------------ //
shared_ptr<BaseGameSpecificPacketFactory> Leviathan::GameSpecificPacketHandler::_FindFactoryForType(int typenumber,
    bool requesttype)
{
	// We can do some optimization if it is sorted //
	if(!IsVectorSorted){
		// Just search the old fashion way //

		for(size_t i = 0; i < AllPacketFactories.size(); i++){
			// Check does it match //
			if(AllPacketFactories[i]->TypeIDNumber == typenumber &&
                AllPacketFactories[i]->HandlesRequests == requesttype)
                
				return AllPacketFactories[i];
		}

		// Nothing found //
		return NULL;
	}

	// The elements are sorted so that the smallest ID is first so search until the id is higher
    // than the wanted one
	for(size_t i = 0; i < AllPacketFactories.size(); i++){
		// Store some data for checking after it doesn't match //
		auto tmpobj = AllPacketFactories[i].get();
		int currenttype = tmpobj->TypeIDNumber;

		// Check does it match //
		if(currenttype == typenumber && tmpobj->HandlesRequests == requesttype)
			return AllPacketFactories[i];

		// Check can we end now //
		if(currenttype > typenumber)
			return NULL;
	}

	// Nothing found //
	return NULL;
}
// ------------------------------------ //
bool SharedPtrVecSortComparison(const std::shared_ptr<BaseGameSpecificPacketFactory> &first,
    const std::shared_ptr<BaseGameSpecificPacketFactory> &second)
{
	return first->TypeIDNumber < second->TypeIDNumber;
}

void Leviathan::GameSpecificPacketHandler::_CheckVectorSorting(){
	// Return if already sorted //
	if(IsVectorSorted)
		return;

	// Sort the vector //
	sort(AllPacketFactories.begin(), AllPacketFactories.end(), &SharedPtrVecSortComparison);


	// Mark as sorted //
	IsVectorSorted = true;
}
// ------------------ BaseGameSpecificRequestPacket ------------------ //
DLLEXPORT Leviathan::BaseGameSpecificRequestPacket::BaseGameSpecificRequestPacket(int typenumber) :
    TypeIDNumber(typenumber)
{

}

DLLEXPORT Leviathan::BaseGameSpecificRequestPacket::~BaseGameSpecificRequestPacket(){

}
// ------------------ BaseGameSpecificResponsePacket ------------------ //
DLLEXPORT Leviathan::BaseGameSpecificResponsePacket::BaseGameSpecificResponsePacket(int typenumber) :
    TypeIDNumber(typenumber)
{

}

DLLEXPORT Leviathan::BaseGameSpecificResponsePacket::~BaseGameSpecificResponsePacket(){

}
// ------------------ GameSpecificPacketData ------------------ //
DLLEXPORT Leviathan::GameSpecificPacketData::GameSpecificPacketData(
    BaseGameSpecificResponsePacket* newddata) :
    IsRequest(false), RequestBaseData(NULL), ResponseBaseData(newddata),
    TypeIDNumber(newddata->TypeIDNumber)
{
	
}

DLLEXPORT Leviathan::GameSpecificPacketData::GameSpecificPacketData(BaseGameSpecificRequestPacket* newddata) :
    IsRequest(true), RequestBaseData(newddata), ResponseBaseData(NULL),
    TypeIDNumber(newddata->TypeIDNumber)
{

}

DLLEXPORT Leviathan::GameSpecificPacketData::~GameSpecificPacketData(){
	SAFE_DELETE(ResponseBaseData);
	SAFE_DELETE(RequestBaseData);
	TypeIDNumber = -1;
}
// ------------------ BaseGameSpecificFactory ------------------ //
DLLEXPORT Leviathan::BaseGameSpecificPacketFactory::BaseGameSpecificPacketFactory(int typenumber, bool isrequesttype) : 
	TypeIDNumber(typenumber), HandlesRequests(isrequesttype)
{
	
}

DLLEXPORT Leviathan::BaseGameSpecificPacketFactory::~BaseGameSpecificPacketFactory(){

}

