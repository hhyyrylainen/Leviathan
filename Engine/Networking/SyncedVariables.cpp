// ------------------------------------ //
#include "SyncedVariables.h"

#include "Common/DataStoring/NamedVars.h"
#include "NetworkRequest.h"
#include "NetworkHandler.h"
#include "Connection.h"
#include "Engine.h"
#include "Common/BaseNotifiable.h"
#include "NetworkClientInterface.h"
#include "Networking/SentNetworkThing.h"
#include "Threading/ThreadingManager.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::SyncedVariables::SyncedVariables(NetworkHandler* owner, 
    NETWORKED_TYPE type, NetworkInterface* handlinginterface) :
    CorrespondingInterface(handlinginterface), Owner(owner), 
    IsHost(type == NETWORKED_TYPE::Server) 
{

}

DLLEXPORT Leviathan::SyncedVariables::~SyncedVariables() {

    ReleaseChildHooks();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::SyncedVariables::AddNewVariable(shared_ptr<SyncedValue> newvalue){
    GUARD_LOCK();

    // Check do we already have a variable with that name //
    if(IsVariableNameUsed(guard, newvalue->GetVariableAccess()->GetName())){
        // Shouldn't add another with the same name //
        return false;
    }

    // Add it //
    Logger::Get()->Info("SyncedVariables: added a new value, "+
        newvalue->GetVariableAccess()->GetName());
    ToSyncValues.push_back(newvalue);

    // Notify update //
    _NotifyUpdatedValue(guard, newvalue.get());

    return true;
}
// ------------------------------------ //
struct SyncedVariables::SendDataSyncAllStruct{
    
    std::vector<shared_ptr<SentNetworkThing>> SentThings;
    bool SentAll {false};
};

void SyncedVariables::_RunSendAllVariablesTask(Connection* connection,
    SyncedVariables* instance, std::shared_ptr<SendDataSyncAllStruct> data)
{
    // Get the loop count //
    // Fetch our object //
    std::shared_ptr<QueuedTask> threadspecific =
        TaskThread::GetThreadSpecificThreadObject()->QuickTaskAccess;
                        
    auto tmpptr =
        dynamic_cast<RepeatCountedDelayedTask*>(
            threadspecific.get());
                                
    LEVIATHAN_ASSERT(tmpptr != NULL,
        "this is not what I wanted, passed wrong task "
        "object to task");

    // Get the value //
    size_t curpos = tmpptr->GetRepeatCount();

    GUARD_LOCK_OTHER(instance);

    if(curpos >= instance->ToSyncValues.size() && curpos >=
        instance->ConnectedChildren.size())
    {

        Logger::Get()->Error("SyncedVariables: queued task "
            "trying to sync variable that is out of vector range");
        return;
    }

    // Sync the value //
    if(curpos < instance->ToSyncValues.size()){
        const SyncedValue* valtosend =
            instance->ToSyncValues[curpos].get();

        // Send it //
        data->SentThings.push_back(
            instance->_SendValueToSingleReceiver(connection,
                valtosend));
    }

    // Sync the resource //
    if(curpos < instance->ConnectedChildren.size()){

        auto tmpvar = static_cast<SyncedResource*>(
            instance->ConnectedChildren[curpos]->
            GetActualPointerToNotifiableObject());

        // Send it //
        data->SentThings.push_back(
            instance->_SendValueToSingleReceiver(
                connection, tmpvar));
    }

    // Check is this the last one //
    if(tmpptr->IsThisLastRepeat()){
        // Set as done //
        data->SentAll = true;
    }
}

DLLEXPORT bool Leviathan::SyncedVariables::HandleSyncRequests(
    shared_ptr<NetworkRequest> request, Connection* connection)
{
    // Switch on the type and see if we can do something with it //
    switch(request->GetType()){
    case NETWORK_REQUEST_TYPE::GetAllSyncValues:
        {
            // Notify that we accepted this //
            // Send the number of values as the string parameter //
            connection->SendPacketToConnection(
                std::make_shared<ResponseServerAllow>(request->GetIDForResponse(), 
                    SERVER_ACCEPTED_TYPE::RequestQueued, Convert::ToString(
                        ToSyncValues.size() + ConnectedChildren.size())),
                RECEIVE_GUARANTEE::Critical);

            auto taskdata = std::make_shared<SendDataSyncAllStruct>();

            // Prepare a task that sends all of the values //
            Engine::Get()->GetThreadingManager()->QueueTask(std::make_shared<
                RepeatCountedDelayedTask>(
                    [=]() -> void 
                    {
                        _RunSendAllVariablesTask(connection, this, taskdata);
                        
                        
                        
                    }, MillisecondDuration(50),
                    MillisecondDuration(10), (int)max(ToSyncValues.size(),
                        ConnectedChildren.size())));
            
            // Queue a finish checking task //
            Engine::Get()->GetThreadingManager()->QueueTask(
                std::make_shared<RepeatingDelayedTask>(
                    [=]() -> void {
                        
                        // Check is it done //
                        if(!taskdata->SentAll)
                            return;

                            // See if all have been sent //
                        for(size_t i = 0; i < taskdata->SentThings.size(); i++){

                            if(!taskdata->SentThings[i]->IsFinalized())
                                    return;
                        }

                        // Stop after this loop //
                        // Fetch our object //
                        std::shared_ptr<QueuedTask> threadspecific =
                            TaskThread::GetThreadSpecificThreadObject()->QuickTaskAccess;
                        auto tmpptr =
                            dynamic_cast<RepeatingDelayedTask*>(threadspecific.get());

                        // Disable the repeating //
                        tmpptr->SetRepeatStatus(false);

                        bool succeeded = true;

                        // Check did some fail //
                        for(size_t i = 0; i < taskdata->SentThings.size(); i++){

                            if(!taskdata->SentThings[i]->GetStatus()){
                                // Failed to send it //

                                succeeded = false;
                                break;
                            }
                        }

                        connection->SendPacketToConnection(
                                std::make_shared<ResponseSyncDataEnd>(0, succeeded), 
                                RECEIVE_GUARANTEE::Critical);

                        }, MillisecondDuration(100), MillisecondDuration(50)));
            
            return true;
        }
        case NETWORK_REQUEST_TYPE::GetSingleSyncValue:
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

DLLEXPORT bool Leviathan::SyncedVariables::HandleResponseOnlySync(
    shared_ptr<NetworkResponse> response, Connection* connection)
{
    // Switch on the type and see if we can do something with it //
    switch(response->GetType()){
    case NETWORK_RESPONSE_TYPE::SyncValData:
        {
            // We got some data that requires syncing //
            if(IsHost){

                Logger::Get()->Warning("SyncedVariables: HandleResponseOnlySync: we are a host "
                    "and got update data, ignoring (use server commands to change "
                    "data on the server)");
                return true;
            }

            // Update the wanted value //
            auto* tmpptr = static_cast<ResponseSyncValData*>(response.get());

            // Call updating function //
            GUARD_LOCK();
            _UpdateFromNetworkReceive(tmpptr, guard);

            return true;
        }
    case NETWORK_RESPONSE_TYPE::SyncResourceData:
        {
            // We got custom sync data //
            if(IsHost){

                Logger::Get()->Warning("SyncedVariables: HandleResponseOnlySync: we are a "
                    "host and got update data, ignoring (use server commands to change "
                    "data on the server)");
                return true;
            }

            auto* tmpptr = static_cast<ResponseSyncResourceData*>(response.get());

            // Create the packet from the data //
            sf::Packet ourdatapacket;
            ourdatapacket.append(tmpptr->OurCustomData.c_str(), tmpptr->OurCustomData.size());

            const std::string lookforname =
                SyncedResource::GetSyncedResourceNameFromPacket(ourdatapacket);

            // Update the one matching the name //
            _OnSyncedResourceReceived(lookforname, ourdatapacket);
            return true;
        }
        case NETWORK_RESPONSE_TYPE::SyncDataEnd:
        {
            // Check if it succeeded or if it failed //
            auto* dataptr = static_cast<ResponseSyncDataEnd*>(response.get());

            if(dataptr->Succeeded){

                Logger::Get()->Info("SyncedVariables: variable sync reported as successful "
                    "by the host");
            } else {

                Logger::Get()->Info("SyncedVariables: variable sync reported as FAILED "
                    "by the host");

                DEBUG_BREAK;
                // Close connection //
                connection->SendCloseConnectionPacket();
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
DLLEXPORT bool Leviathan::SyncedVariables::IsVariableNameUsed(Lock &guard, 
    const std::string &name)
{
    // Loop all and compare their names //
    for(size_t i = 0; i < ToSyncValues.size(); i++){
        if(ToSyncValues[i]->GetVariableAccess()->CompareName(name))
            return true;
    }

    // Didn't match any names //
    return false;
}
// ------------------------------------ //
void SyncedVariables::_NotifyUpdatedValue(Lock &guard, const SyncedValue* const valtosync,
    int useid /*= -1*/)
{
    // Create an update packet //

    auto tmpresponse = std::make_shared<ResponseSyncValData>(useid,
        *valtosync->GetVariableAccess());

    const auto& connections = Owner->GetInterface()->GetClientConnections();

    // Send it //
    for(auto& Connection : connections){

        // Send to connection //
        Connection->SendPacketToConnection(tmpresponse, RECEIVE_GUARANTEE::Critical);
    }
}

void Leviathan::SyncedVariables::_NotifyUpdatedValue(Lock &guard, SyncedResource* valtosync, 
    int useid /*= -1*/)
{
    // Only update if we are a host //
    if(!IsHost)
        return;

    // Serialize it to a packet //
    sf::Packet packet;

    valtosync->AddDataToPacket(guard, packet);

    auto tmpresponse = std::make_shared<ResponseSyncResourceData>(0, 
        std::string(reinterpret_cast<const char*>(packet.getData()), packet.getDataSize()));

    const auto& connections = Owner->GetInterface()->GetClientConnections();

    // Send it //
    for (auto& Connection : connections) {

        // Send to connection //
        Connection->SendPacketToConnection(tmpresponse, RECEIVE_GUARANTEE::Critical);
    }
}
// ------------------------------------ //
shared_ptr<SentNetworkThing> Leviathan::SyncedVariables::_SendValueToSingleReceiver(
    Connection* unsafeptr, const SyncedValue* const valtosync)
{
    // Create an update packet //
    return unsafeptr->SendPacketToConnection(std::make_shared<ResponseSyncValData>(
            0, *valtosync->GetVariableAccess()), RECEIVE_GUARANTEE::Critical);
}

shared_ptr<SentNetworkThing> Leviathan::SyncedVariables::_SendValueToSingleReceiver(
    Connection* unsafeptr, SyncedResource* valtosync)
{
    sf::Packet packet;

    valtosync->AddDataToPacket(packet);

    return unsafeptr->SendPacketToConnection(std::make_shared<ResponseSyncResourceData>(0,
            std::string(reinterpret_cast<const char*>(packet.getData()),
                packet.getDataSize())), RECEIVE_GUARANTEE::Critical);
}
// ------------------------------------ //
void Leviathan::SyncedVariables::_OnSyncedResourceReceived(const std::string &name,
    sf::Packet &packetdata)
{
    GUARD_LOCK();

    // Search through our SyncedResources //
    for(size_t i = 0; i < ConnectedChildren.size(); i++){
        // Check does it match the name //
        auto tmpvar = static_cast<SyncedResource*>(
            ConnectedChildren[i]->GetActualPointerToNotifiableObject());

        if(tmpvar->Name == name){

            // Unlock to allow BaseNotifiable to lock us //
            guard.unlock();
            
            // It is this //
            tmpvar->UpdateDataFromPacket(packetdata);

            guard.lock();

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
void Leviathan::SyncedVariables::_UpdateFromNetworkReceive(ResponseSyncValData* datatouse, 
    Lock &guard)
{
    LEVIATHAN_ASSERT(!IsHost, 
        "Hosts cannot received value updates by others, use server side commands");

    // Get the data from it //
    const NamedVariableList& data = datatouse->SyncValueData;

    // Match a variable with the name //
    for(size_t i = 0; i < ToSyncValues.size(); i++){

        NamedVariableList* tmpaccess = ToSyncValues[i]->GetVariableAccess();

        if(tmpaccess->CompareName(data.GetName())){

            // Update the value //
            if(*tmpaccess == data){

                Logger::Get()->Info("SyncedVariables: no need to update variable "+ data.GetName());
                return;
            }

            // Set it //
            *tmpaccess = data;

            // Do some updating if we are doing a full sync //
            if(!SyncDone){

                _UpdateReceiveCount(data.GetName());
            }
        }
    }

    // Add a new variable //
    Logger::Get()->Info("SyncedVariables: adding a new variable, because value for it was received, "+
        data.GetName());

    ToSyncValues.push_back(shared_ptr<SyncedValue>(new SyncedValue(
        new NamedVariableList(data), true, true)));

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
    ValueNamesUpdated.push_back(unique_ptr<std::string>(new std::string(nameofthing)));

    // Increment count and notify //
    ++ActualGotThingCount;

    auto iface = Owner->GetClientInterface();
    if(iface)
        iface->OnUpdateFullSynchronizationState(ActualGotThingCount, ExpectedThingCount);
}

DLLEXPORT void Leviathan::SyncedVariables::SetExpectedNumberOfVariablesReceived(size_t amount){
    ExpectedThingCount = amount;
}
// ------------------ SyncedValue ------------------ //
DLLEXPORT Leviathan::SyncedValue::SyncedValue(NamedVariableList* newddata,
    bool passtoclients /*= true*/, bool allowevents /*= true*/) : 
    PassToClients(passtoclients), AllowSendEvents(allowevents), HeldVariables(newddata)
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
