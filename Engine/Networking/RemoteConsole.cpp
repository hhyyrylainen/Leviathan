// ------------------------------------ //
#include "RemoteConsole.h"

#include "Connection.h"
#include "Application/Application.h"
#include "../TimeIncludes.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::RemoteConsole::RemoteConsole() : CloseIfNoRemoteConsole(false), 
    CanClose(false)
{
}

DLLEXPORT Leviathan::RemoteConsole::~RemoteConsole(){
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::UpdateStatus(){

    GUARD_LOCK();
    // Check awaiting connections //
    auto timenow = Time::GetThreadSafeSteadyTimePoint();

    for(size_t i = 0; i < AwaitingConnections.size(); i++){
        if(AwaitingConnections[i]->TimeoutTime < timenow){
            // Time it out //
            Logger::Get()->Warning("RemoteConsole: Remote console wait connection timed out, "
                "token "+Convert::ToString(AwaitingConnections[i]->SessionToken));
            
            AwaitingConnections.erase(AwaitingConnections.begin()+i);
            i--;
            continue;
        }
    }

    // Special checks //
    if(CloseIfNoRemoteConsole && CanClose){
        // Send close to application if no connection (or waiting for one) //
        if(AwaitingConnections.size() == 0 && RemoteConsoleConnections.size() == 0){
            // Time to close //

            Logger::Get()->Info("RemoteConsole: closing the program because "
                "CloseIfNoRemoteConsole, and no active connections");
            LeviathanApplication::Get()->MarkAsClosing();
        }
    }

    for(auto iter = RemoteConsoleConnections.begin(); iter != RemoteConsoleConnections.end(); )
    {
        if((*iter)->TerminateSession && !(*iter)->GetConnection()->IsValidForSend()){

            Logger::Get()->Info("RemoteConsole: removing kill-queued session, token: "+
                Convert::ToString((*iter)->SessionToken));
            iter = RemoteConsoleConnections.erase(iter);
        } else {
            ++iter;
        }
    }

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RemoteConsole::IsAwaitingConnections(){
    return AwaitingConnections.size() != 0;
}

DLLEXPORT void Leviathan::RemoteConsole::ExpectNewConnection(int SessionToken,
    const std::string &assignname /*= ""*/, bool onlylocalhost /*= false*/,
    const MillisecondDuration &timeout /*= std::chrono::seconds(30)*/)
{
    GUARD_LOCK();

    AwaitingConnections.push_back(shared_ptr<RemoteConsoleExpect>(
            new RemoteConsoleExpect(assignname, SessionToken,
                onlylocalhost, timeout)));
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RemoteConsole::CanOpenNewConnection(
    std::shared_ptr<Connection> connection,
    std::shared_ptr<NetworkRequest> request)
{
    // Get data from the packet //
    bool local = connection->IsTargetHostLocalhost();

    switch (request->GetType()) {
    case NETWORK_REQUEST_TYPE::DoRemoteConsoleOpen:
    {
        DEBUG_BREAK;
        return true;
    }
    case NETWORK_REQUEST_TYPE::RemoteConsoleOpen:
    {
        auto* opennew = static_cast<RequestRemoteConsoleOpen*>(request.get());

        int sessiontoken = opennew->SessionToken;

        // Look for a matching awaiting connection //
        for (size_t i = 0; i < AwaitingConnections.size(); i++) {
            if ((AwaitingConnections[i]->OnlyLocalhost && local) ||
                !AwaitingConnections[i]->OnlyLocalhost)
            {
                if (AwaitingConnections[i]->SessionToken == sessiontoken) {
                    // Match found //
                    Logger::Get()->Info("RemoteConsole: matching connection request got!");

                    // Add to real connections //
                    RemoteConsoleConnections.push_back(std::make_shared<RemoteConsoleSession>(
                        AwaitingConnections[i]->ConnectionName,
                        connection, AwaitingConnections[i]->SessionToken));

                    // Open new, send succeed packet back //
                    connection->SendPacketToConnection(std::make_shared<ResponseNone>(
                            NETWORK_RESPONSE_TYPE::RemoteConsoleOpened,
                            request->GetIDForResponse()), RECEIVE_GUARANTEE::Critical);

                    AwaitingConnections.erase(AwaitingConnections.begin() + i);
                    return true;
                }
            }
        }
    }
    default:
        return false;
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::OfferConnectionTo(
    std::shared_ptr<Connection> connectiontouse,
    const std::string &connectionname, int token)
{
    GUARD_LOCK();

    // Add to the expected connections //
    AwaitingConnections.push_back(shared_ptr<RemoteConsoleExpect>(
            new RemoteConsoleExpect(connectionname, token,
                connectiontouse->IsTargetHostLocalhost(), std::chrono::seconds(15))));

    // Send a request that the target connects to us //
    connectiontouse->SendPacketToConnection(std::make_shared<RequestDoRemoteConsoleOpen>(
            token), RECEIVE_GUARANTEE::Critical);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::HandleRemoteConsoleRequestPacket(
    std::shared_ptr<NetworkRequest> request,
    std::shared_ptr<Connection> connection)
{
    // First check if it should be handled by CanOpenNewConnection which handless 
    // all open connection packets //
    if(request->GetType() == NETWORK_REQUEST_TYPE::DoRemoteConsoleOpen || request->GetType() ==
        NETWORK_REQUEST_TYPE::RemoteConsoleOpen)
    {
        CanOpenNewConnection(connection, request);
        return;
    }

    GUARD_LOCK();

    // Handle normal RemoteConsole request //
    switch(request->GetType()){
    case NETWORK_REQUEST_TYPE::CloseRemoteConsole:
        {
            // Kill connection //
            GetRemoteConsoleSessionForConnection(guard, *connection)->KillConnection();
            Logger::Get()->Info("RemoteConsole: closing connection due to close request");

            connection->SendPacketToConnection(std::make_shared<ResponseNone>(
                    NETWORK_RESPONSE_TYPE::RemoteConsoleClosed,
                    request->GetIDForResponse()), RECEIVE_GUARANTEE::Critical);
            return;
        }
    default:
        LOG_ERROR("RemoteConsole unhandled request type");        
    }

    DEBUG_BREAK;
}

DLLEXPORT void Leviathan::RemoteConsole::HandleRemoteConsoleResponse(
    std::shared_ptr<NetworkResponse> response,
    Connection &connection, std::shared_ptr<NetworkRequest> potentialrequest)
{
    // We can detect close messages //
    switch(response->GetType()){
    case NETWORK_RESPONSE_TYPE::RemoteConsoleOpened: 
    case NETWORK_RESPONSE_TYPE::RemoteConsoleClosed:
        {
            // These shouldn't be received //
            Logger::Get()->Warning("RemoteConsole: HandleRemoteConsoleResponse: got a "
                "packet of type remote opened/"
                "remote closed, not expected to be received without us requesting it");
        }
        break;
    default:
        Logger::Get()->Warning("RemoteConsole: HandleRemoteConsoleResponse: got a "
            "packet of unknown type, "
            "maybe this should not have been passed to RemoteConsole");
        DEBUG_BREAK;
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::SetCloseIfNoRemoteConsole(bool state){
    CloseIfNoRemoteConsole = state;
}

DLLEXPORT RemoteConsoleSession* Leviathan::RemoteConsole::GetRemoteConsoleSessionForConnection(
    Lock &guard, Connection &connection)
{
    // Loop over and compare pointers //
    for(size_t i = 0; i < RemoteConsoleConnections.size(); i++){
        if(RemoteConsoleConnections[i]->GetConnection() == &connection){
            // Found a matching one //
            return RemoteConsoleConnections[i].get();
        }
    }

    return nullptr;
}
// ------------------------------------ //
DLLEXPORT size_t Leviathan::RemoteConsole::GetActiveConnectionCount(){
    return RemoteConsoleConnections.size();
}

DLLEXPORT std::shared_ptr<Leviathan::Connection> 
Leviathan::RemoteConsole::GetConnectionForRemoteConsoleSession(const std::string &name) 
{
    for (auto& connection : RemoteConsoleConnections) {
        if (connection->ConnectionName == name) {

            return connection->CorrespondingConnection;
        }
    }

    return nullptr;
}
// ------------------------------------ //
void Leviathan::RemoteConsole::SetAllowClose(){
    CanClose = true;
}
// ------------------ RemoteConsoleExpect ------------------ //
Leviathan::RemoteConsole::RemoteConsoleExpect::RemoteConsoleExpect(const std::string &name,
    int token, bool onlylocalhost, const MillisecondDuration &timeout) :
    ConnectionName(name), SessionToken(token), OnlyLocalhost(onlylocalhost),
    TimeoutTime(Time::GetThreadSafeSteadyTimePoint()+timeout)
{

}
// ------------------ RemoteConsoleSession ------------------ //
Leviathan::RemoteConsoleSession::~RemoteConsoleSession(){
    // Send close request //
    if(CorrespondingConnection->IsValidForSend()){

        CorrespondingConnection->SendPacketToConnection(std::make_shared<RequestNone>(
                NETWORK_REQUEST_TYPE::CloseRemoteConsole), 
            RECEIVE_GUARANTEE::Critical);
    }
}
// ------------------------------------ //
DLLEXPORT Connection* Leviathan::RemoteConsoleSession::GetConnection(){
    return CorrespondingConnection.get();
}

DLLEXPORT void Leviathan::RemoteConsoleSession::KillConnection(){
    TerminateSession = true;
}
