// ------------------------------------ //
#include "SyncedResource.h"

#include "SyncedVariables.h"
#include "Exceptions.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::SyncedResource::SyncedResource(const std::string &uniquename) : Name(uniquename){

}

DLLEXPORT Leviathan::SyncedResource::~SyncedResource(){
	// Unregister (if not already done) //
	ReleaseParentHooks();
}

DLLEXPORT void Leviathan::SyncedResource::StartSync(){
	// The SyncedVariables might be unloaded at this point //
	auto sync = SyncedVariables::Get();

	if(!sync)
		return;

	// Register us //
	ConnectToNotifier(sync);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedResource::UpdateDataFromPacket(sf::Packet &packet){
	GUARD_LOCK();
    
	// Load the custom data //
	try{
		UpdateCustomDataFromPacket(guard, packet);

	} catch(const InvalidArgument &e){

		e.PrintToLog();
		return false;
	}

	// Notify us about the update //
	OnValueUpdated(guard);

	// Notify listeners //
	NotifyAll(guard);
	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedResource::NotifyUpdatedValue(){
	GUARD_LOCK();

	// Update the networked value //
	UpdateOurNetworkValue(guard);

	// Notify us about the update //
	OnValueUpdated(guard);

	// Notify listeners //
	NotifyAll(guard);
}
// ------------------------------------ //
void Leviathan::SyncedResource::OnValueUpdated(Lock &guard){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedResource::AddDataToPacket(sf::Packet &packet){
	GUARD_LOCK();
	// First add the name //
	packet << Name;

	// Then add our data //
	SerializeCustomDataToPacket(guard, packet);
}

DLLEXPORT std::string Leviathan::SyncedResource::GetSyncedResourceNameFromPacket(sf::Packet &packet){
	// Get the name from the packet //
	std::string tmpstr;

	packet >> tmpstr;
		
    if(!packet)
		throw InvalidArgument("resource sync packet has invalid format");        

	return tmpstr;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedResource::UpdateOurNetworkValue(){
	GUARD_LOCK();
    
    // TODO: proper locking
	auto synman = SyncedVariables::Get();
    if(synman)
        synman->_NotifyUpdatedValue(this);
}

