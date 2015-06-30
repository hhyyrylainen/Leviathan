// ------------------------------------ //
#include "RemoteConsole.h"

#include "ConnectionInfo.h"
#include "Application/Application.h"
#include "../TimeIncludes.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT Leviathan::RemoteConsole::RemoteConsole() : CloseIfNoRemoteConsole(false), CanClose(false){
	staticinstance = this;
}

DLLEXPORT Leviathan::RemoteConsole::~RemoteConsole(){
	staticinstance = NULL;
}

DLLEXPORT RemoteConsole* Leviathan::RemoteConsole::Get(){
	return staticinstance;
}

RemoteConsole* Leviathan::RemoteConsole::staticinstance = NULL;
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
			LeviathanApplication::GetApp()->MarkAsClosing();
		}
	}

	for(auto iter = RemoteConsoleConnections.begin(); iter != RemoteConsoleConnections.end(); ){
		if((*iter)->TerminateSession){

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

DLLEXPORT void Leviathan::RemoteConsole::ExpectNewConnection(int SessionToken, const std::string &assignname /*= L""*/,
    bool onlylocalhost /*= false*/, const MillisecondDuration &timeout /*= boost::chrono::seconds(30)*/)
{
	GUARD_LOCK();

	AwaitingConnections.push_back(shared_ptr<RemoteConsoleExpect>(new RemoteConsoleExpect(assignname, SessionToken,
                onlylocalhost, timeout)));
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RemoteConsole::CanOpenNewConnection(ConnectionInfo* connection,
    std::shared_ptr<NetworkRequest> request)
{
	// Get data from the packet //
	bool local = connection->IsTargetHostLocalhost();

	// Get the token from the packet //
	auto tmpdata = request->GetRemoteConsoleOpenToData();

	auto opennew = request->GetRemoteConsoleAccessRequestData();

	if(!tmpdata && !opennew){
		// Invalid package data/type //
		return false;
	}

	int sessiontoken = tmpdata ? tmpdata->SessionToken: opennew->SessionToken;

	// Look for a matching awaiting connection //
	for(size_t i = 0; i < AwaitingConnections.size(); i++){
		if((AwaitingConnections[i]->OnlyLocalhost && local) ||
            !AwaitingConnections[i]->OnlyLocalhost)
        {
			if(AwaitingConnections[i]->SessionToken == sessiontoken){
				// Match found //
				Logger::Get()->Info("RemoteConsole: matching connection request got!");

				// Add to real connections //
				RemoteConsoleConnections.push_back(shared_ptr<RemoteConsoleSession>(
                        new RemoteConsoleSession(AwaitingConnections[i]->ConnectionName,
                            connection, AwaitingConnections[i]->SessionToken)));

				// Link us with the lifetime of the connection //
				connection->ConnectToNotifiable(this);

				if(tmpdata){
					// Create a open request //
					shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(
                            new RemoteConsoleAccessRequestData(
                                AwaitingConnections[i]->SessionToken)));

					// Send initial packet //
					connection->SendPacketToConnection(tmprequest, 3);
					WaitingRequests.push_back(tmprequest);

					// We should also reply with an empty response //
					shared_ptr<NetworkResponse> tmpresponse(
                        new NetworkResponse(request->GetExpectedResponseID(),
                            PACKET_TIMEOUT_STYLE_TIMEDMS, 5000));
					tmpresponse->GenerateEmptyResponse();

					connection->SendPacketToConnection(tmpresponse, 5);

				} else if(opennew){
					// Open new, send succeed packet back //
					shared_ptr<NetworkResponse> tmpresponse(
                        new NetworkResponse(request->GetExpectedResponseID(),
                            PACKET_TIMEOUT_STYLE_TIMEDMS, 1000));
					tmpresponse->GenerateRemoteConsoleOpenedResponse();

					connection->SendPacketToConnection(tmpresponse, 5);
				}

				AwaitingConnections.erase(AwaitingConnections.begin()+i);
				return true;
			}
		}
	}

	// Didn't find a match //
	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::OfferConnectionTo(ConnectionInfo* connectiontouse,
    const std::string &connectionname, int token)
{
	GUARD_LOCK();
	// Add to the expected connections //
	AwaitingConnections.push_back(shared_ptr<RemoteConsoleExpect>(
            new RemoteConsoleExpect(connectionname, token,
                connectiontouse->IsTargetHostLocalhost(), std::chrono::seconds(15))));

	// Send a request that the target connects to us //
	shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(
            new RemoteConsoleOpenRequestDataTo(token)));


	connectiontouse->SendPacketToConnection(tmprequest, 4);
}
// ------------------------------------ //
void Leviathan::RemoteConsole::_OnNotifierDisconnected(Lock &guard,
    BaseNotifierAll* parenttoremove, Lock &parentlock)
{
	// Close the corresponding console session //

	Logger::Get()->Info("RemoteConsole: detected connection closing, trying to close matching remote "
        "console session:");

	for(size_t i = 0; i < RemoteConsoleConnections.size(); i++){
		if(RemoteConsoleConnections[i]->GetConnection() == parenttoremove){
			// Close it //
			Logger::Get()->Info("\t> RemoteConsole: closed matching connection");
			RemoteConsoleConnections[i]->ResetConnection();
			RemoteConsoleConnections.erase(RemoteConsoleConnections.begin()+i);
			return;
		}
	}
    
	// Not found //
	Logger::Get()->Error("\t> RemoteConsole: didn't find a matching connection, bug?");
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::HandleRemoteConsoleRequestPacket(shared_ptr<NetworkRequest> request,
    ConnectionInfo* connection)
{
	// First check if it should be handled by CanOpenNewConnection which handless all open connection packets //
	if(request->GetType() == NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE || request->GetType() ==
        NETWORKREQUESTTYPE_OPENREMOTECONSOLETO)
    {

		CanOpenNewConnection(connection, request);
		return;
	}

	GUARD_LOCK();

	// Handle normal RemoteConsole request //
	switch(request->GetType()){
	case NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE:
		{
			// Kill connection //
			GetRemoteConsoleSessionForConnection(connection, guard)->KillConnection();
			Logger::Get()->Info("RemoteConsole: closing connection due to close request");
		}
		break;
	default:
		DEBUG_BREAK;

	}
}

DLLEXPORT void Leviathan::RemoteConsole::HandleRemoteConsoleResponse(shared_ptr<NetworkResponse> response,
    ConnectionInfo* connection, std::shared_ptr<NetworkRequest> potentialrequest)
{
	// We can detect close messages //
	switch(response->GetTypeOfResponse()){
	case NETWORKRESPONSETYPE_REMOTECONSOLECLOSED: case NETWORKRESPONSETYPE_REMOTECONSOLEOPENED:
		{
			// These shouldn't be received //
			Logger::Get()->Warning("RemoteConsole: HandleRemoteConsoleResponse: got a packet of type remote opened/"
                "remote closed, not expected to be received without us requesting it");
		}
		break;
	default:
		Logger::Get()->Warning("RemoteConsole: HandleRemoteConsoleResponse: got a packet of unknown type, "
            "maybe this should not have been passed to RemoteConsole");
	}
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::SetCloseIfNoRemoteConsole(bool state){
	CloseIfNoRemoteConsole = state;
}

DLLEXPORT RemoteConsoleSession* Leviathan::RemoteConsole::GetRemoteConsoleSessionForConnection(
    ConnectionInfo* connection, Lock &guard)
{
	VerifyLock(guard);
	// Loop over and compare pointers //
	for(size_t i = 0; i < RemoteConsoleConnections.size(); i++){
		if(RemoteConsoleConnections[i]->GetConnection() == connection){
			// Found a matching one //
			return RemoteConsoleConnections[i].get();
		}
	}

	// Didn't find a matching one //
	return NULL;
}

DLLEXPORT size_t Leviathan::RemoteConsole::GetActiveConnectionCount(){
	return RemoteConsoleConnections.size();
}

DLLEXPORT ConnectionInfo* Leviathan::RemoteConsole::GetUnsafeConnectionForRemoteConsoleSession(const std::string &name){
	GUARD_LOCK();
	// Loop over and compare names //
	for(size_t i = 0; i < RemoteConsoleConnections.size(); i++){
		if(RemoteConsoleConnections[i]->ConnectionName == name){
			// Found a matching one //
			return RemoteConsoleConnections[i]->GetConnection();
		}
	}

	// Didn't find a matching one //
	return NULL;
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
Leviathan::RemoteConsoleSession::RemoteConsoleSession(const std::string &name, ConnectionInfo* connection, int token) :
    ConnectionName(name), SessionToken(token), CorrespondingConnection(connection), TerminateSession(false),
    IsOpened(true)
{

}

Leviathan::RemoteConsoleSession::~RemoteConsoleSession(){
	// Send close request //
	if(CorrespondingConnection){


		auto safe = NetworkHandler::Get()->GetSafePointerToConnection(CorrespondingConnection);

		if(safe){

			shared_ptr<NetworkRequest> tmprequest(
                new NetworkRequest(NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE));

			safe->SendPacketToConnection(tmprequest, 1);
		}
	}

}
// ------------------------------------ //
DLLEXPORT ConnectionInfo* Leviathan::RemoteConsoleSession::GetConnection(){
	return CorrespondingConnection;
}

DLLEXPORT void Leviathan::RemoteConsoleSession::ResetConnection(){
	// Set our connection to NULL //
	CorrespondingConnection = NULL;
}

DLLEXPORT void Leviathan::RemoteConsoleSession::KillConnection(){
	TerminateSession = true;
}
