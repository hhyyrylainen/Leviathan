#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_SYNCEDVARIABLES
#include "SyncedVariables.h"
#endif
#include "Common/DataStoring/NamedVars.h"
#include "NetworkRequest.h"
#include "NetworkHandler.h"
#include "ConnectionInfo.h"
#include "Engine.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::SyncedVariables::SyncedVariables(NetworkHandler* owner, bool amiaserver, NetworkInterface* handlinginterface) : 
	IsHost(amiaserver), CorrespondingInterface(handlinginterface), Owner(owner), SyncDone(false)
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
	GUARD_LOCK_THIS_OBJECT();

	// Check do we already have a variable with that name //
	if(IsVariableNameUsed(newvalue->GetVariableAccess()->GetName())){
		// Shouldn't add another with the same name //
		return false;
	}

	// Add it //
	Logger::Get()->Info(L"SyncedVariables: added a new value, "+newvalue->GetVariableAccess()->GetName());
	ToSyncValues.push_back(newvalue);

	// Notify update //
	_NotifyUpdatedValue(newvalue.get());

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedVariables::AddAnotherToSyncWith(ConnectionInfo* unsafeptr){
	GUARD_LOCK_THIS_OBJECT();

	// Report it //
	for(auto iter = ConnectedToOthers.begin(); iter != ConnectedToOthers.end(); ++iter){
		if(*iter == unsafeptr){

			Logger::Get()->Warning(L"SyncedVariables: AddAnotherToSyncWith: already connected to the specified one (could try to get the address here...)");
			return;
		}
	}

	// Add the new one //
	ConnectedToOthers.push_back(unsafeptr);
	Logger::Get()->Warning(L"SyncedVariables: AddAnotherToSyncWith: connected to a new other one (could try to get the address here...)");
	return;
}

DLLEXPORT void Leviathan::SyncedVariables::RemoveConnectionWithAnother(ConnectionInfo* unsafeptr){
	GUARD_LOCK_THIS_OBJECT();

	// Look for a matching pointer and remove it //
	for(auto iter = ConnectedToOthers.begin(); iter != ConnectedToOthers.end(); ++iter){
		if(*iter == unsafeptr){

			Logger::Get()->Warning(L"SyncedVariables: RemoveConnectionWithAnother: removed a connection (could try to get the address here...)");
			ConnectedToOthers.erase(iter);
			return;
		}
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedVariables::HandleSyncRequests(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	// Switch on the type and see if we can do something with it //
	switch(request->GetType()){
	case NETWORKREQUESTTYPE_GETALLSYNCVALUES:
		{
			// Notify that we accepted this //
			shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 1000));

			tmpresponse->GenerateServerAllowResponse(new NetworkResponseDataForServerAllow(NETWORKRESPONSE_SERVERACCEPTED_TYPE_REQUEST_QUEUED, L"-1"));

			connection->SendPacketToConnection(tmpresponse, 5);

			struct SendDataSyncAllStruct{
				SendDataSyncAllStruct() : SentAll(false){};

				std::vector<shared_ptr<SentNetworkThing>> SentThings;
				bool SentAll;

			};

			shared_ptr<SendDataSyncAllStruct> taskdata(new SendDataSyncAllStruct());

			// Prepare a task that sends all of the values //
			Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new RepeatCountedDelayedTask(
				boost::bind<void>([](ConnectionInfo* connection, SyncedVariables* instance, shared_ptr<SendDataSyncAllStruct> data) -> void 
			{
				// Get the loop count //
				// Fetch our object //
				shared_ptr<QueuedTask> threadspecific = TaskThread::GetThreadSpecificThreadObject()->QuickTaskAccess;
				auto tmpptr = dynamic_cast<RepeatCountedDelayedTask*>(threadspecific.get());
				assert(tmpptr != NULL && "this is not what I wanted, passed wrong task object to task");

				int repeat = tmpptr->GetRepeatCount();

				// Get the value //
				size_t curpos = (size_t)repeat;

				if(curpos >= instance->ToSyncValues.size()){

					Logger::Get()->Error(L"SyncedVariables: queued task trying to sync variable that is out of vector range");
					return;
				}

				GUARD_LOCK_OTHER_OBJECT(instance);

				// Sync the value //
				const SyncedValue* valtosend = instance->ToSyncValues[curpos].get();

				// Send it //
				data->SentThings.push_back(instance->_SendValueToSingleReceiver(connection, valtosend));

				// Check is this the last one //
				if(tmpptr->IsThisLastRepeat()){
					// Set as done //
					data->SentAll = true;
					Logger::Get()->Info(L"SyncedVariables: sent all variables");
				}


			}, connection, this, taskdata), MillisecondDuration(100), MillisecondDuration(10), (int)ToSyncValues.size())));

			// Queue a finish checking task //
			Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new RepeatingDelayedTask(
				boost::BOOST_BIND<void>([](ConnectionInfo* connection, SyncedVariables* instance, shared_ptr<SendDataSyncAllStruct> data) -> void
			{
				// Check is it done //
				if(!data->SentAll)
					return;

				// See if all have been sent //
				for(size_t i = 0; i < data->SentThings.size(); i++){

					if(!data->SentThings[i]->GetFutureForThis().has_value())
						return;
				}

				// Stop after this loop //
				// Fetch our object //
				shared_ptr<QueuedTask> threadspecific = TaskThread::GetThreadSpecificThreadObject()->QuickTaskAccess;
				auto tmpptr = dynamic_cast<RepeatingDelayedTask*>(threadspecific.get());
				assert(tmpptr != NULL && "this is not what I wanted, passed wrong task object to task");

				// Disable the repeating //
				tmpptr->SetRepeatStatus(false);

				unique_ptr<NetworkResponseDataForSyncDataEnd> tmpresponddata;

				// Check did some fail //
				for(size_t i = 0; i < data->SentThings.size(); i++){

					if(!data->SentThings[i]->GetFutureForThis().get()){
						// Failed to send it //

						tmpresponddata = unique_ptr<NetworkResponseDataForSyncDataEnd>(new NetworkResponseDataForSyncDataEnd(false));
						break;
					}
				}

				// It succeeded (if not set already) //
				if(!tmpresponddata)
					tmpresponddata = unique_ptr<NetworkResponseDataForSyncDataEnd>(new NetworkResponseDataForSyncDataEnd(true));

				// Send response //
				shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(-1, PACKAGE_TIMEOUT_STYLE_TIMEDMS, 3000));
				tmpresponse->GenerateValueSyncEndResponse(tmpresponddata.release());

				shared_ptr<ConnectionInfo> safeconnection = NetworkHandler::Get()->GetSafePointerToConnection(connection);

				safeconnection->SendPacketToConnection(tmpresponse, 3);

			}, connection, this, taskdata), MillisecondDuration(1000), MillisecondDuration(250))));

			return true;
		}
	case NETWORKREQUESTTYPE_GETSINGLESYNCVALUE:
		{
			// Send the value //
			DEBUG_BREAK;
			return true;
		}
	}

	// Could not process //
	return false;
}

DLLEXPORT bool Leviathan::SyncedVariables::HandleResponseOnlySync(shared_ptr<NetworkResponse> response, ConnectionInfo* connection){
	// Switch on the type and see if we can do something with it //
	switch(response->GetType()){
	case NETWORKRESPONSETYPE_SYNCVALDATA:
		{
			// We got some data that requires syncing //
			if(IsHost){

				Logger::Get()->Warning(L"SyncedVariables: HandleResponseOnlySync: we are a host and got update data, ignoring "
					L"(use server commands to change data on the server)");
				return true;
			}

			// Update the wanted value //
			auto tmpptr = response->GetResponseDataForValueSyncResponse();

			if(!tmpptr){

				Logger::Get()->Error(L"SyncedVariables: received a response containing no variable data");
				return true;
			}

			// Call updating function //
			GUARD_LOCK_THIS_OBJECT();
			_UpdateFromNetworkReceive(tmpptr, guard);
			return true;
		}
	case NETWORKRESPONSETYPE_SYNCDATAEND:
		{
			// Check if it succeeded or if it failed //
			auto dataptr = response->GetResponseDataForValueSyncEndResponse();

			if(dataptr->Succeeded){

				Logger::Get()->Info(L"SyncedVariables: variable sync reported as successful by the host");
			} else {

				Logger::Get()->Info(L"SyncedVariables: variable sync reported as FAILED by the host");
			}

			// Mark sync as ended //
			SyncDone = true;

			return true;
		}
	}

	// Could not process //
	return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedVariables::IsVariableNameUsed(const wstring &name, ObjectLock &guard){
	VerifyLock(guard);

	// Loop all and compare their names //
	for(size_t i = 0; i < ToSyncValues.size(); i++){
		if(ToSyncValues[i]->GetVariableAccess()->CompareName(name))
			return true;
	}

	// Didn't match any names //
	return false;
}
// ------------------------------------ //
void Leviathan::SyncedVariables::_NotifyUpdatedValue(const SyncedValue* const valtosync, int useid /*= -1*/){
	// Create an update packet //
	shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(useid, PACKAGE_TIMEOUT_STYLE_TIMEDMS, 3000));

	tmpresponse->GenerateValueSyncResponse(new NetworkResponseDataForSyncValData(new NamedVariableList(*valtosync->GetVariableAccess())));

	// Report //
	Logger::Get()->Info(L"SyncedVariables: syncing variable "+valtosync->GetVariableAccess()->GetName()+L" with everyone");

	GUARD_LOCK_THIS_OBJECT();

	// Send it //
	for(size_t i = 0; i < ConnectedToOthers.size(); i++){
		// The connection might have closed so we need to retrieve the actual pointer //
		auto safeptr = NetworkHandler::Get()->GetSafePointerToConnection(ConnectedToOthers[i]);

		if(safeptr){
			// Send to connection //
			safeptr->SendPacketToConnection(tmpresponse, 100);

		} else {
			DEBUG_BREAK;
		}
	}
}

shared_ptr<SentNetworkThing> Leviathan::SyncedVariables::_SendValueToSingleReceiver(ConnectionInfo* unsafeptr, const SyncedValue* const valtosync){
	// Create an update packet //
	shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(-1, PACKAGE_TIMEOUT_STYLE_TIMEDMS, 3000));

	tmpresponse->GenerateValueSyncResponse(new NetworkResponseDataForSyncValData(new NamedVariableList(*valtosync->GetVariableAccess())));

	// Report //
	Logger::Get()->Info(L"SyncedVariables: syncing variable "+valtosync->GetVariableAccess()->GetName()+L" with specific");

	// The connection might have closed so we need to retrieve the actual pointer //
	auto safeptr = NetworkHandler::Get()->GetSafePointerToConnection(unsafeptr);

	if(safeptr){
		// Send to connection //
		return safeptr->SendPacketToConnection(tmpresponse, 5);

	}

	DEBUG_BREAK;
	return nullptr;
}
// ------------------------------------ //
void Leviathan::SyncedVariables::_UpdateFromNetworkReceive(NetworkResponseDataForSyncValData* datatouse, ObjectLock &guard){
	assert(!IsHost && "Hosts cannot received value updates by others, use server side commands");
	VerifyLock(guard);

	// Get the data from it //
	NamedVariableList* tmpptr = datatouse->SyncValueData.get();

	// Match a variable with the name //
	for(size_t i = 0; i < ToSyncValues.size(); i++){

		NamedVariableList* tmpaccess = ToSyncValues[i]->GetVariableAccess();

		if(tmpaccess->CompareName(tmpptr->GetName())){

			// Update the value //
			if(*tmpaccess == *tmpptr){

				Logger::Get()->Info(L"SyncedVariables: no need to update variable "+tmpptr->GetName());
				return;
			}

			// Set it //
			*tmpaccess = *tmpptr;
		}
	}

	// Add a new variable //
	Logger::Get()->Info(L"SyncedVariables: adding a new variable, because value for it was received, "+tmpptr->GetName());

	ToSyncValues.push_back(shared_ptr<SyncedValue>(new SyncedValue(new NamedVariableList(*tmpptr), true, true)));
	ToSyncValues.back()->_MasterYouCalled(this);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedVariables::PrepareForFullSync(){
	SyncDone = false;
}

DLLEXPORT bool Leviathan::SyncedVariables::IsSyncDone(){
	return SyncDone;
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
	// Send to the Owner for doing stuff //
	Owner->_NotifyUpdatedValue(this);
}
// ------------------------------------ //
DLLEXPORT NamedVariableList* Leviathan::SyncedValue::GetVariableAccess() const{
	return HeldVariables;
}
// ------------------------------------ //
void Leviathan::SyncedValue::_MasterYouCalled(SyncedVariables* owner){
	Owner = owner;
}
