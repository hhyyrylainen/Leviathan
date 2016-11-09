// ------------------------------------ //
#include "SyncedResource.h"

#include "SyncedVariables.h"
#include "Exceptions.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::SyncedResource::SyncedResource(const std::string &uniquename) : 
    Name(uniquename)
{

}

DLLEXPORT Leviathan::SyncedResource::~SyncedResource(){

    GUARD_LOCK();

	// Unregister (if not already done) //
	ReleaseParentHooks(guard);
}

DLLEXPORT void Leviathan::SyncedResource::StartSync(SyncedVariables &variablesync)
{
	ConnectToNotifier(&variablesync);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedResource::UpdateDataFromPacket(Lock &guard, 
    sf::Packet &packet)
{
    
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
DLLEXPORT void Leviathan::SyncedResource::AddDataToPacket(Lock &guard, sf::Packet &packet){

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
DLLEXPORT void Leviathan::SyncedResource::UpdateOurNetworkValue(Lock &guard){
    
    if (ConnectedToParents.size() < 1) {

        LOG_WARNING("Updating SyncedResource that isn't attached to SyncedVariables");
        return;
    }

    static_cast<SyncedVariables*>(ConnectedToParents[0]->GetActualPointerToNotifierObject())
        ->_NotifyUpdatedValue(guard, this);
}

