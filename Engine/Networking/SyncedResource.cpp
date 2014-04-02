#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SYNCEDRESOURCE
#include "SyncedResource.h"
#endif
#include "SyncedVariables.h"
#include "Exceptions\ExceptionInvalidArgument.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::SyncedResource::SyncedResource(const wstring &uniquename) : Name(uniquename){

}

DLLEXPORT Leviathan::SyncedResource::~SyncedResource(){
	// Unregister (if not already done) //
	ReleaseParentHooks();
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
	return true;
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
	SyncedVariables::Get()->_NotifyUpdatedValue(this);
}

