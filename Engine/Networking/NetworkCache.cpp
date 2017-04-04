// ------------------------------------ //
#include "NetworkCache.h"

#include "Networking/NetworkHandler.h"
#include "Networking/NetworkResponse.h"
#include "Threading/ThreadingManager.h"
#include "Networking/Connection.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT NetworkCache::NetworkCache(NETWORKED_TYPE serverside) : 
    IsServer(serverside == NETWORKED_TYPE::Server)
{

}

DLLEXPORT NetworkCache::~NetworkCache(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkCache::Init(NetworkHandler* owner) {

    Owner = owner;

    LEVIATHAN_ASSERT(Owner, "NetworkCache no owner");
    return Owner != nullptr;
}

DLLEXPORT void NetworkCache::Release() {

    GUARD_LOCK();

    CurrentVariables.clear();
}
// ------------------------------------ //
DLLEXPORT bool NetworkCache::UpdateVariable(const NamedVariableList &updatedvalue){

    GUARD_LOCK();

    if(!IsServer)
        return false;
    
    // Remove old variable //
    auto end = CurrentVariables.end();
    for(auto iter = CurrentVariables.begin(); iter != end; ++iter){

        if(updatedvalue.CompareName((*iter)->GetName())){

            // Check does the value match //
            if(updatedvalue == *(*iter)){

                // The value is the same //
                return true;
            }
            
            CurrentVariables.erase(iter);
            break;
        }
    }

    // Add it //
    CurrentVariables.push_back(std::make_shared<NamedVariableList>(updatedvalue));
    
    _OnVariableUpdated(guard, updatedvalue);

    return true;
}
        
DLLEXPORT bool NetworkCache::RemoveVariable(const std::string &name){

    GUARD_LOCK();

    auto end = CurrentVariables.end();
    for(auto iter = CurrentVariables.begin(); iter != end; ++iter){

        if((*iter)->GetName() == name){

            CurrentVariables.erase(iter);
            Logger::Get()->Info("NetworkCache: todo: send remove message to clients");
            return true;
        }
    }

    return false;
}
// ------------------------------------ //
DLLEXPORT bool NetworkCache::HandleUpdatePacket(ResponseCacheUpdated* data){

    GUARD_LOCK();

    // Remove old variable (if any) //
    for(auto iter = CurrentVariables.begin(); iter != CurrentVariables.end(); ++iter){

        if(data->Variable.CompareName((*iter)->GetName())){

            CurrentVariables.erase(iter);
            break;
        }
    }

    // Add it //
    CurrentVariables.push_back(std::make_shared<NamedVariableList>(data->Variable));

    return true;
}

DLLEXPORT bool Leviathan::NetworkCache::HandleUpdatePacket(ResponseCacheRemoved* data) {

    DEBUG_BREAK;
    return true;
}

// ------------------------------------ //
DLLEXPORT NamedVariableList* NetworkCache::GetVariable(const std::string &name) const{

    GUARD_LOCK();

    for(auto iter = CurrentVariables.begin(); iter != CurrentVariables.end(); ++iter){

        if((*iter)->GetName() == name){

            return (*iter).get();
        }
    }

    return nullptr;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkCache::_OnNewConnection(
    std::shared_ptr<Connection> connection) 
{
    if(!IsServer)
        return;

    if(!connection->IsValidForSend())
        return;

    // TODO: make sure this doesn't use a deleted object
    ThreadingManager::Get()->QueueTask(new QueuedTask(std::bind<void>([](NetworkCache* cache,
        std::shared_ptr<Connection> connection)
        -> void
    {

        size_t vars = 0;

        {
            GUARD_LOCK_OTHER(cache);
            vars = cache->CurrentVariables.size();
        }

        for (size_t index = 0; index < vars; index++) {

            std::shared_ptr<NamedVariableList> target;

            {
                GUARD_LOCK_OTHER(cache);
                if(index >= cache->CurrentVariables.size())
                    return;

                target = cache->CurrentVariables[index];
            }

            if(!target)
                continue;

            connection->SendPacketToConnection(std::make_shared<ResponseCacheUpdated>(0,
                    *target), RECEIVE_GUARANTEE::Critical);
        }

    }, this, connection)));
}
// ------------------------------------ //
void Leviathan::NetworkCache::_OnVariableUpdated(Lock &guard, 
    const NamedVariableList &variable) 
{
    auto& connections = Owner->GetInterface()->GetClientConnections();

    for (auto& connection : connections) {

        connection->SendPacketToConnection(std::make_shared<ResponseCacheUpdated>(0, variable),
            RECEIVE_GUARANTEE::Critical);
    }
}
// ------------------------------------ //
DLLEXPORT ScriptSafeVariableBlock* NetworkCache::GetVariableWrapper(const std::string &name){

    auto variable = GetVariable(name);

    if(!variable)
        return nullptr;

    auto value = variable->GetValueDirect();

    if(!value)
        return nullptr;

    return new ScriptSafeVariableBlock(value, variable->GetName());
}

DLLEXPORT void NetworkCache::SetVariableWrapper(ScriptSafeVariableBlock* variable){

    UpdateVariable(NamedVariableList(variable->GetName(),
            variable->GetBlockConst()->AllocateNewFromThis()));
}
