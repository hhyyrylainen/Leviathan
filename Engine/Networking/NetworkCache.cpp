// ------------------------------------ //
#include "NetworkCache.h"

#include "Networking/NetworkHandler.h"
#include "Networking/NetworkResponse.h"
#include "Threading/ThreadingManager.h"
#include "Networking/ConnectionInfo.h"
#include "boost/bind.hpp"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT NetworkCache::NetworkCache(bool serverside) : IsServer(serverside){

    Staticinstance = this;
}

DLLEXPORT NetworkCache::~NetworkCache(){

    Staticinstance = NULL;
}

NetworkCache* NetworkCache::Staticinstance = NULL;

DLLEXPORT NetworkCache* NetworkCache::Get(){

    return Staticinstance;
}
// ------------------------------------ //
DLLEXPORT bool NetworkCache::Init(){

    GUARD_LOCK();
    
    return true;
}

DLLEXPORT void NetworkCache::Release(){

    GUARD_LOCK();

    ReceivingConnections.clear();

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
    CurrentVariables.push_back(make_shared<NamedVariableList>(updatedvalue));
    
    _OnVariableUpdated(CurrentVariables.back(), guard);

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
DLLEXPORT bool NetworkCache::HandleUpdatePacket(NetworkResponseDataForAICacheUpdated* data){

    GUARD_LOCK();

    if(!data->Variable)
        return false;

    // Remove old variable (if any) //
    auto end = CurrentVariables.end();
    for(auto iter = CurrentVariables.begin(); iter != end; ++iter){

        if(data->Variable->CompareName((*iter)->GetName())){

            CurrentVariables.erase(iter);
            break;
        }
    }

    // Add it //
    CurrentVariables.push_back(data->Variable);

    return true;
}
// ------------------------------------ //
DLLEXPORT NamedVariableList* NetworkCache::GetVariable(const std::string &name) const{

    GUARD_LOCK();

    auto end = CurrentVariables.end();
    for(auto iter = CurrentVariables.begin(); iter != end; ++iter){

        if((*iter)->GetName() == name){

            return (*iter).get();
        }
    }

    return nullptr;
}
// ------------------------------------ //
DLLEXPORT bool NetworkCache::RegisterNewConnection(ConnectionInfo* connection){

    GUARD_LOCK();

    if(!IsServer)
        return false;

    auto safeptr = NetworkHandler::Get()->GetSafePointerToConnection(connection);

    if(!safeptr)
        return false;
    
    
    ReceivingConnections.push_back(connection);

    
    ThreadingManager::Get()->QueueTask(new QueuedTask(boost::bind<void>([](NetworkCache* cache,
                    std::shared_ptr<ConnectionInfo> connection)
                -> void
        {

            size_t vars = 0;

            {
                GUARD_LOCK_OTHER(cache);
                vars = cache->CurrentVariables.size();
            }

            for(size_t index = 0; index < vars; index++){

                std::shared_ptr<NamedVariableList> target;

                {
                    GUARD_LOCK_OTHER(cache);
                    if(index >= cache->CurrentVariables.size())
                        return;

                    target = cache->CurrentVariables[index];
                }

                if(!target)
                    continue;
                
                auto response = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 5);

                response->GenerateAICacheUpdatedResponse(new NetworkResponseDataForAICacheUpdated(
                        target));

                connection->SendPacketToConnection(response, 20);
            }
            
        }, this, safeptr)));

    return true;
}

DLLEXPORT bool NetworkCache::RemoveConnection(ConnectionInfo* connection){

    GUARD_LOCK();

    auto end = ReceivingConnections.end();
    for(auto iter = ReceivingConnections.begin(); iter != end; ++iter){

        if((*iter) == connection){

            ReceivingConnections.erase(iter);
            return true;
        }
    }

    return false;
}
// ------------------------------------ //
void NetworkCache::_OnVariableUpdated(shared_ptr<NamedVariableList> variable, Lock &guard){

    if(ReceivingConnections.empty())
        return;
    
    ThreadingManager::Get()->QueueTask(new QueuedTask(boost::bind<void>([](NetworkCache* cache,
                    std::shared_ptr<NamedVariableList> variable)
                -> void
        {

            ConnectionInfo* target = NULL;

            auto response = make_shared<NetworkResponse>(-1, PACKET_TIMEOUT_STYLE_PACKAGESAFTERRECEIVED, 5);

            response->GenerateAICacheUpdatedResponse(new
                NetworkResponseDataForAICacheUpdated(variable));
            

            while(true){
                
                bool changedtarget = false;

                // Update target //
                {
                    // Loop until the last one is found and then change to the next //
                    bool found = target ? false: true;
                    
                    GUARD_LOCK_OTHER(cache);
                    
                    for(size_t i = 0; i < cache->ReceivingConnections.size(); i++){

                        if(found){

                            target = cache->ReceivingConnections[i];
                            changedtarget = true;
                            break;
                        }

                        if(cache->ReceivingConnections[i] == target)
                            found = true;
                    }
                }
                
                if(!changedtarget)
                    return;

                // Send to the target //
                if(!target)
                    return;

                auto safe = NetworkHandler::Get()->GetSafePointerToConnection(target);

                if(safe)
                    safe->SendPacketToConnection(response, 20);
            }

            


        }, this, variable)));
}
// ------------------------------------ //
DLLEXPORT ScriptSafeVariableBlock* NetworkCache::GetVariableWrapper(const string &name){

    auto variable = GetVariable(name);

    if(!variable)
        return NULL;

    auto value = variable->GetValueDirect();

    if(!value)
        return NULL;

    return new ScriptSafeVariableBlock(value, variable->GetName());
}

DLLEXPORT void NetworkCache::SetVariableWrapper(ScriptSafeVariableBlock* variable){

    UpdateVariable(NamedVariableList(variable->GetName(),
            variable->GetBlockConst()->AllocateNewFromThis()));
}
