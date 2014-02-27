#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SYNCEDVARIABLES
#include "SyncedVariables.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::SyncedVariables::SyncedVariables(NetworkHandler* owner, bool amiaserver, NetworkInterface* handlinginterface) : 
	IsHost(amiaserver), CorrespondingInterface(handlinginterface), Owner(owner)
{
	Staticaccess = this;
}

DLLEXPORT Leviathan::SyncedVariables::~SyncedVariables(){
	Staticaccess = NULL;
}

DLLEXPORT SyncedVariables* Leviathan::SyncedVariables::Get(){
	return Staticaccess;
}

SyncedVariables* Leviathan::SyncedVariables::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedVariables::AddNewVariable(shared_ptr<SyncedValue> newvalue){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedVariables::AddAnotherToSyncWith(ConnectionInfo* unsafeptr){

}

DLLEXPORT void Leviathan::SyncedVariables::RemoveConnectionWithAnother(ConnectionInfo* unsafeptr){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedVariables::HandleSyncRequests(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){

}

DLLEXPORT bool Leviathan::SyncedVariables::HandleResponseOnlySync(shared_ptr<NetworkResponse> response, ConnectionInfo* connection){

}
// ------------------ SyncedValue ------------------ //
DLLEXPORT Leviathan::SyncedValue::SyncedValue(NamedVariableList* newddata, bool passtoclients /*= true*/, bool allowevents /*= true*/) : 
	Owner(NULL), PassToClients(passtoclients), AllowSendEvents(allowevents), HeldVariables(newddata){

}

DLLEXPORT Leviathan::SyncedValue::~SyncedValue(){
	// Release the data //
	SAFE_DELETE(HeldVariables);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedValue::NotifyUpdated(){
	// Send to the Owner for doing stuff
}
// ------------------------------------ //
DLLEXPORT NamedVariableList* Leviathan::SyncedValue::GetVariableAccess() const{
	return HeldVariables;
}
// ------------------------------------ //
bool Leviathan::SyncedValue::_MasterYouCalled(SyncedVariables* owner){
	Owner = owner;
}
