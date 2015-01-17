#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SYNCEDRESOURCE
#include "SyncedResource.h"
#endif
#include "SyncedVariables.h"
#include "Exceptions/ExceptionInvalidArgument.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::SyncedResource::SyncedResource(const wstring &uniquename) : Name(uniquename){
	// Now we need to be registered later //
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
	GUARD_LOCK_THIS_OBJECT();
	// Load the custom data //
	try{
		UpdateCustomDataFromPacket(packet);

	} catch(const ExceptionInvalidArgument &e){
		// Something failed //
		e.PrintToLog();
		return false;
	}

	// Notify us about the update //
	OnValueUpdated();

	// Notify listeners //
	NotifyAll();
	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedResource::NotifyUpdatedValue(){
	// Update the networked value //
	UpdateOurNetworkValue();

	GUARD_LOCK_THIS_OBJECT();

	// Notify us about the update //
	OnValueUpdated();

	// Notify listeners //
	NotifyAll();
}
// ------------------------------------ //
void Leviathan::SyncedResource::OnValueUpdated(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedResource::AddDataToPacket(sf::Packet &packet){
	GUARD_LOCK_THIS_OBJECT();
	// First add the name //
	packet << Name;

	// Then add our data //
	SerializeCustomDataToPacket(packet);
}

DLLEXPORT wstring Leviathan::SyncedResource::GetSyncedResourceNameFromPacket(sf::Packet &packet) THROWS{
	// Get the name from the packet //
	wstring tmpstr;

	if(!(packet >> tmpstr)){
		
		throw ExceptionInvalidArgument(L"resource sync packet has invalid format", 0, __WFUNCTION__, L"packet", L"");
	}

	return tmpstr;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedResource::UpdateOurNetworkValue(){
	GUARD_LOCK_THIS_OBJECT();
    // TODO: proper locking
	auto synman = SyncedVariables::Get();
    if(synman)
        synman->_NotifyUpdatedValue(this);
}

