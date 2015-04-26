// ------------------------------------ //
#include "SyncedVariables.h"

#include "Common/DataStoring/NamedVars.h"
#include "NetworkRequest.h"
#include "NetworkHandler.h"
#include "ConnectionInfo.h"
#include "Engine.h"
#include "Common/BaseNotifiable.h"
#include "NetworkClientInterface.h"
#include "Threading/ThreadingManager.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::SyncedVariables::SyncedVariables(NetworkHandler* owner, bool amiaserver,
    NetworkInterface* handlinginterface) :
	IsHost(amiaserver), CorrespondingInterface(handlinginterface), Owner(owner), SyncDone(false),
    ExpectedThingCount(0), ActualGotThingCount(0)
{
	Staticaccess = this;
}

DLLEXPORT Leviathan::SyncedVariables::~SyncedVariables(){
	Staticaccess = NULL;

	ReleaseParentHooks();
	ReleaseChildHooks();
}

DLLEXPORT SyncedVariables* Leviathan::SyncedVariables::Get(){
	return Staticaccess;
}

SyncedVariables* Leviathan::SyncedVariables::Staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedVariables::AddNewVariable(shared_ptr<SyncedValue> newvalue){
	GUARD_LOCK();

	// Check do we already have a variable with that name //
	if(IsVariableNameUsed(newvalue->GetVariableAccess()->GetName())){
		// Shouldn't add another with the same name //
		return false;
	}

	// Add it //
	Logger::Get()->Info("SyncedVariables: added a new value, "+newvalue->GetVariableAccess()->GetName());
	ToSyncValues.push_back(newvalue);

	// Notify update //
	_NotifyUpdatedValue(newvalue.get());

	return true;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedVariables::AddAnotherToSyncWith(ConnectionInfo* unsafeptr){
	GUARD_LOCK();

	// Report it //
	for(auto iter = ConnectedToOthers.begin(); iter != ConnectedToOthers.end(); ++iter){
		if(*iter == unsafeptr){

			Logger::Get()->Info("SyncedVariables: AddAnotherToSyncWith: already connected to the"
                "specified one (could try to get the address here...)");
			return;
		}
	}

	ConnectToNotifier(unsafeptr);

	// Add the new one //
	ConnectedToOthers.push_back(unsafeptr);
	Logger::Get()->Warning("SyncedVariables: AddAnotherToSyncWith: connected to a new other one"
        "(could try to get the address here...)");
	return;
}

DLLEXPORT void Leviathan::SyncedVariables::RemoveConnectionWithAnother(ConnectionInfo* ptr,
    Lock &guard, bool alreadyunhooking)
{
	VerifyLock(guard);

	// Look for a matching pointer and remove it //
	for(auto iter = ConnectedToOthers.begin(); iter != ConnectedToOthers.end(); ++iter){
		if(*iter == ptr){

			Logger::Get()->Info("SyncedVariables: RemoveConnectionWithAnother: removed a connection");
			ConnectedToOthers.erase(iter);

			if(!alreadyunhooking)
				UnConnectFromNotifier(ptr);
			return;
		}
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedVariables::HandleSyncRequests(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connection)
{
	// Switch on the type and see if we can do something with it //
	switch(request->GetType()){
        case NETWORKREQUESTTYPE_GETALLSYNCVALUES:
		{
			// Notify that we accepted this //
			shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(),
                    PACKET_TIMEOUT_STYLE_TIMEDMS, 1000));

			// Send the number of values as the string parameter //
			tmpresponse->GenerateServerAllowResponse(new NetworkResponseDataForServerAllow(
                    NETWORKRESPONSE_SERVERACCEPTED_TYPE_REQUEST_QUEUED,
                    Convert::ToStd::String(ToSyncValues.size()+ConnectedChildren.size())));

			connection->SendPacketToConnection(tmpresponse, 5);

			struct SendDataSyncAllStruct{
				SendDataSyncAllStruct() : SentAll(false){};

				std::vector<shared_ptr<SentNetworkThing>> SentThings;
				bool SentAll;

			};

			shared_ptr<SendDataSyncAllStruct> taskdata(new SendDataSyncAllStruct());

			// Prepare a task that sends all of the values //
			Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new RepeatCountedDelayedTask(
                        boost::bind<void>([](ConnectionInfo* connection, SyncedVariables* instance,
                                std::shared_ptr<SendDataSyncAllStruct> data) -> void 
                            {
                                // Get the loop count //
                                // Fetch our object //
                                std::shared_ptr<QueuedTask> threadspecific =
                                    TaskThread::GetThreadSpecificThreadObject()->QuickTaskAccess;
                
                                auto tmpptr = dynamic_cast<RepeatCountedDelayedTask*>(threadspecific.get());
                                assert(tmpptr != NULL && "this is not what I wanted, passed wrong task object to task");

                                int repeat = tmpptr->GetRepeatCount();

                                // Get the value //
                                size_t curpos = (size_t)repeat;

                                GUARD_LOCK_OTHER(instance);

                                if(curpos >= instance->ToSyncValues.size() && curpos >=
                                    instance->ConnectedChildren.size())
                                {

                                    Logger::Get()->Error("SyncedVariables: queued task trying to sync variable that is out of vector range");
                                    return;
                                }

                                // Sync the value //
                                if(curpos < instance->ToSyncValues.size()){
                                    const SyncedValue* valtosend = instance->ToSyncValues[curpos].get();

                                    // Send it //
                                    data->SentThings.push_back(instance->_SendValueToSingleReceiver(connection,
                                            valtosend));
                                }

                                // Sync the resource //
                                if(curpos < instance->ConnectedChildren.size()){

                                    auto tmpvar = static_cast<SyncedResource*>(instance->ConnectedChildren[curpos]->
                                        GetActualPointerToNotifiableObject());

                                    // Send it //
                                    data->SentThings.push_back(instance->_SendValueToSingleReceiver(
                                            connection, tmpvar));
                                }

                                // Check is this the last one //
                                if(tmpptr->IsThisLastRepeat()){
                                    // Set as done //
                                    data->SentAll = true;
                                }


                            }, connection, this, taskdata), MillisecondDuration(50),
                        MillisecondDuration(10), (int)max(ToSyncValues.size(), ConnectedChildren.size()))));

			// Queue a finish checking task //
			Engine::Get()->GetThreadingManager()->QueueTask(shared_ptr<QueuedTask>(new RepeatingDelayedTask(
                        boost::BOOST_BIND<void>([](ConnectionInfo* connection, SyncedVariables*
                                instance, std::shared_ptr<SendDataSyncAllStruct> data) -> void
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
                                std::shared_ptr<QueuedTask> threadspecific =
                                    TaskThread::GetThreadSpecificThreadObject()->QuickTaskAccess;
                                auto tmpptr = dynamic_cast<RepeatingDelayedTask*>(threadspecific.get());
                                assert(tmpptr != NULL && "this is not what I wanted, passed wrong task object to task");

                                // Disable the repeating //
                                tmpptr->SetRepeatStatus(false);

                                std::unique_ptr<NetworkResponseDataForSyncDataEnd> tmpresponddata;

                                // Check did some fail //
                                for(size_t i = 0; i < data->SentThings.size(); i++){

                                    if(!data->SentThings[i]->GetFutureForThis().get()){
                                        // Failed to send it //

                                        tmpresponddata = std::unique_ptr<NetworkResponseDataForSyncDataEnd>(new
                                            NetworkResponseDataForSyncDataEnd(false));
                                        break;
                                    }
                                }

                                // It succeeded (if not set already) //
                                if(!tmpresponddata)
                                    tmpresponddata = std::unique_ptr<NetworkResponseDataForSyncDataEnd>(new
                                        NetworkResponseDataForSyncDataEnd(true));

                                // Send response //
                                std::shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(-1,
                                        PACKET_TIMEOUT_STYLE_TIMEDMS, 3000));
                                tmpresponse->GenerateValueSyncEndResponse(tmpresponddata.release());

                                std::shared_ptr<ConnectionInfo> safeconnection =
                                    NetworkHandler::Get()->GetSafePointerToConnection(connection);

                                safeconnection->SendPacketToConnection(tmpresponse, 3);

                            }, connection, this, taskdata), MillisecondDuration(100), MillisecondDuration(50))));

			return true;
		}
        case NETWORKREQUESTTYPE_GETSINGLESYNCVALUE:
		{
			// Send the value //
			DEBUG_BREAK;
			return true;
		}
        default:
            return false;
	}

	// Could not process //
	return false;
}

DLLEXPORT bool Leviathan::SyncedVariables::HandleResponseOnlySync(shared_ptr<NetworkResponse> response, ConnectionInfo*
    connection)
{
	// Switch on the type and see if we can do something with it //
	switch(response->GetType()){
        case NETWORKRESPONSETYPE_SYNCVALDATA:
		{
			// We got some data that requires syncing //
			if(IsHost){

				Logger::Get()->Warning("SyncedVariables: HandleResponseOnlySync: we are a host and got update data, "
                    "ignoring (use server commands to change data on the server)");
				return true;
			}

			// Update the wanted value //
			auto tmpptr = response->GetResponseDataForValueSyncResponse();

			if(!tmpptr){

				Logger::Get()->Error("SyncedVariables: received a response containing no variable data");
				return true;
			}

			// Call updating function //
			GUARD_LOCK();
			_UpdateFromNetworkReceive(tmpptr, guard);

			return true;
		}
        case NETWORKRESPONSETYPE_SYNCRESOURCEDATA:
		{
			// We got custom sync data //
			if(IsHost){

                Logger::Get()->Warning("SyncedVariables: HandleResponseOnlySync: we are a host and got update data, "
                    "ignoring (use server commands to change data on the server)");
				return true;
			}


			NetworkResponseDataForSyncResourceData* data = response->GetResponseDataForSyncResourceResponse();
			if(!data){
				Logger::Get()->Error("SyncedVariables: received a resource sync response containing no data");
				return true;
			}

			// Create the packet from the data //
			sf::Packet ourdatapacket;
			ourdatapacket.append(data->OurCustomData.c_str(), data->OurCustomData.size());

			const std::string lookforname = SyncedResource::GetSyncedResourceNameFromPacket(ourdatapacket);

			// Update the one matching the name //
			_OnSyncedResourceReceived(lookforname, ourdatapacket);
			return true;
		}
        case NETWORKRESPONSETYPE_SYNCDATAEND:
		{
			// Check if it succeeded or if it failed //
			auto dataptr = response->GetResponseDataForValueSyncEndResponse();

			if(dataptr->Succeeded){

				Logger::Get()->Info("SyncedVariables: variable sync reported as successful by the host");
			} else {

				Logger::Get()->Info("SyncedVariables: variable sync reported as FAILED by the host");
			}

			// Mark sync as ended //
			SyncDone = true;

			return true;
		}
        default:
            return false;
	}

	// Could not process //
	return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedVariables::IsVariableNameUsed(const std::string &name, Lock &guard){
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
	shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(useid, PACKET_TIMEOUT_STYLE_TIMEDMS, 3000));

	tmpresponse->GenerateValueSyncResponse(new NetworkResponseDataForSyncValData(new
            NamedVariableList(*valtosync->GetVariableAccess())));

	GUARD_LOCK();

	// Send it //
	for(size_t i = 0; i < ConnectedToOthers.size(); i++){

		// Send to connection //
		ConnectedToOthers[i]->SendPacketToConnection(tmpresponse, 100);
	}
}

void Leviathan::SyncedVariables::_NotifyUpdatedValue(SyncedResource* valtosync, int useid /*= -1*/){
	// Only update if we are a host //
	if(!IsHost)
		return;

	// Create an update packet //
	shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 3000));

	// Serialize it to a packet //
	sf::Packet packet;

	valtosync->AddDataToPacket(packet);

	// Extract it from the packet //
	tmpresponse->GenerateResourceSyncResponse(reinterpret_cast<const char*>(packet.getData()), packet.getDataSize());

	GUARD_LOCK();

	// Send it //
	for(size_t i = 0; i < ConnectedToOthers.size(); i++){

		// Send to connection //
		ConnectedToOthers[i]->SendPacketToConnection(tmpresponse, 100);
	}
}
// ------------------------------------ //
shared_ptr<SentNetworkThing> Leviathan::SyncedVariables::_SendValueToSingleReceiver(ConnectionInfo* unsafeptr, const
    SyncedValue* const valtosync)
{
	// Create an update packet //
	shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 3000));

	tmpresponse->GenerateValueSyncResponse(new NetworkResponseDataForSyncValData(new
            NamedVariableList(*valtosync->GetVariableAccess())));

	// Send to connection //
	return unsafeptr->SendPacketToConnection(tmpresponse, 5);
}

shared_ptr<SentNetworkThing> Leviathan::SyncedVariables::_SendValueToSingleReceiver(ConnectionInfo* unsafeptr,
    SyncedResource* valtosync)
{
	// Create an update packet //
	shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(-1, PACKET_TIMEOUT_STYLE_TIMEDMS, 3000));

	// Serialize it to a packet //
	sf::Packet packet;

	valtosync->AddDataToPacket(packet);

	// Extract it from the packet //
	tmpresponse->GenerateResourceSyncResponse(reinterpret_cast<const char*>(packet.getData()), packet.getDataSize());

	// Send to connection //
	return unsafeptr->SendPacketToConnection(tmpresponse, 5);
}
// ------------------------------------ //
void Leviathan::SyncedVariables::_OnSyncedResourceReceived(const std::string &name, sf::Packet &packetdata){
	GUARD_LOCK();

	// Search through our SyncedResources //
	for(size_t i = 0; i < ConnectedChildren.size(); i++){
		// Check does it match the name //
		auto tmpvar = static_cast<SyncedResource*>(ConnectedChildren[i]->GetActualPointerToNotifiableObject());

		if(tmpvar->Name == name){
			// It is this //
			tmpvar->UpdateDataFromPacket(packetdata);

			// Do some updating if we are doing a full sync //
			if(!SyncDone){

				_UpdateReceiveCount(name);
			}
			return;
		}
	}
	Logger::Get()->Warning("SyncedVariables: synced resource with the name \""+name+"\" was not found/updated");
}
// ------------------------------------ //
void Leviathan::SyncedVariables::_UpdateFromNetworkReceive(NetworkResponseDataForSyncValData* datatouse, Lock
    &guard)
{
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

				Logger::Get()->Info("SyncedVariables: no need to update variable "+tmpptr->GetName());
				return;
			}

			// Set it //
			*tmpaccess = *tmpptr;

			// Do some updating if we are doing a full sync //
			if(!SyncDone){

				_UpdateReceiveCount(tmpptr->GetName());
			}
		}
	}

	// Add a new variable //
	Logger::Get()->Info("SyncedVariables: adding a new variable, because value for it was received, "+
        tmpptr->GetName());

	ToSyncValues.push_back(shared_ptr<SyncedValue>(new SyncedValue(new NamedVariableList(*tmpptr), true, true)));
	ToSyncValues.back()->_MasterYouCalled(this);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::SyncedVariables::PrepareForFullSync(){
	// Reset some variables //
	SyncDone = false;
	ExpectedThingCount = 0;
	ValueNamesUpdated.clear();
	ActualGotThingCount = 0;
}

DLLEXPORT bool Leviathan::SyncedVariables::IsSyncDone(){
	return SyncDone;
}

void Leviathan::SyncedVariables::_UpdateReceiveCount(const std::string &nameofthing){
	// Check is it already updated (values can update while a sync is being done) //
	for(size_t i = 0; i < ValueNamesUpdated.size(); i++){

		if(*ValueNamesUpdated[i] == nameofthing)
			return;
	}

	// Add it //
	ValueNamesUpdated.push_back(move(unique_ptr<std::string>(new std::string(nameofthing))));

	// Increment count and notify //
	++ActualGotThingCount;


	auto iface = NetworkClientInterface::GetIfExists();
	if(iface)
		iface->OnUpdateFullSynchronizationState(ActualGotThingCount, ExpectedThingCount);
}

DLLEXPORT void Leviathan::SyncedVariables::SetExpectedNumberOfVariablesReceived(size_t amount){
	ExpectedThingCount = amount;
}

void Leviathan::SyncedVariables::_OnNotifierDisconnected(BaseNotifierAll* parenttoremove){
	GUARD_LOCK();

	Logger::Get()->Info("SyncedVariables: stopping sync with specific, because connection is closing");
	RemoveConnectionWithAnother(static_cast<ConnectionInfo*>(parenttoremove), guard, true);
}
// ------------------ SyncedValue ------------------ //
DLLEXPORT Leviathan::SyncedValue::SyncedValue(NamedVariableList* newddata, bool passtoclients /*= true*/,
    bool allowevents /*= true*/) : 
	Owner(NULL), PassToClients(passtoclients), AllowSendEvents(allowevents), HeldVariables(newddata)
{

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
