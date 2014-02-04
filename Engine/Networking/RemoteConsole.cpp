#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_REMOTECONSOLE
#include "RemoteConsole.h"
#endif
#include "ConnectionInfo.h"
#include "Application\Application.h"
#include "Common\Misc.h"
using namespace Leviathan;
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
	ObjectLock guard(*this);
	// Check awaiting connections //
	auto timenow = Misc::GetThreadSafeSteadyTimePoint();

	for(size_t i = 0; i < AwaitingConnections.size(); i++){
		if(AwaitingConnections[i]->TimeoutTime < timenow){
			// Time it out //
			Logger::Get()->Warning(L"RemoteConsole: Remote console wait connection timed out, token "+Convert::ToWstring(AwaitingConnections[i]->SessionToken));
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

			Logger::Get()->Info(L"RemoteConsole: closing the program because CloseIfNoRemoteConsole, and no active connections");
			LeviathanApplication::GetApp()->StartRelease();
		}
	}

	for(auto iter = RemoteConsoleConnections.begin(); iter != RemoteConsoleConnections.end(); ){
		if((*iter)->TerminateSession){

			Logger::Get()->Info(L"RemoteConsole: removing kill-queued session, token: "+Convert::ToWstring((*iter)->SessionToken));
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

DLLEXPORT void Leviathan::RemoteConsole::ExpectNewConnection(int SessionToken, const wstring &assignname /*= L""*/, bool onlylocalhost /*= false*/, 
	const MillisecondDuration &timeout /*= boost::chrono::seconds(30)*/)
{
	ObjectLock guard(*this);

	AwaitingConnections.push_back(shared_ptr<RemoteConsoleExpect>(new RemoteConsoleExpect(assignname, SessionToken, onlylocalhost, timeout)));
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RemoteConsole::CanOpenNewConnection(ConnectionInfo* connection, shared_ptr<NetworkRequest> request){
	// Get data from the packet //
	bool local = connection->IsTargetHostLocalhost();

	// Get the token from the packet //
	auto tmpdata = request->GetRemoteConsoleOpenToDataIfPossible();

	auto opennew = request->GetRemoteConsoleAccessRequestDataIfPossible();

	if(!tmpdata && !opennew){
		// Invalid package data/type //
		return false;
	}

	int sessiontoken = tmpdata ? tmpdata->SessionToken: opennew->SessionToken;

	// Look for a matching awaiting connection //
	for(size_t i = 0; i < AwaitingConnections.size(); i++){
		if(AwaitingConnections[i]->OnlyLocalhost && local || !AwaitingConnections[i]->OnlyLocalhost){
			if(AwaitingConnections[i]->SessionToken == sessiontoken){
				// Match found //
				Logger::Get()->Info(L"RemoteConsole: matching connection request got!");

				// Add to real connections //
				RemoteConsoleConnections.push_back(shared_ptr<RemoteConsoleSession>(new RemoteConsoleSession(AwaitingConnections[i]->ConnectionName,
					connection, AwaitingConnections[i]->SessionToken)));

				// Link us with the lifetime of the connection //
				connection->ConnectToNotifiable(this);

				if(tmpdata){
					// Create a open request //
					shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(new RemoteConsoleAccessRequestData(AwaitingConnections[i]->SessionToken)));

					// Send initial packet //
					connection->SendPacketToConnection(tmprequest, 3);
					WaitingRequests.push_back(tmprequest);

					// We should also reply with an empty response //
					shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 5000));
					tmpresponse->GenerateEmptyResponse();

					connection->SendPacketToConnection(tmpresponse, 5);

				} else if(opennew){
					// Open new, send succeed packet back //
					shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 1000));
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
DLLEXPORT void Leviathan::RemoteConsole::OfferConnectionTo(ConnectionInfo* connectiontouse, const wstring &connectionname, int token){
	ObjectLock guard(*this);
	// Add to the expected connections //
	AwaitingConnections.push_back(shared_ptr<RemoteConsoleExpect>(new RemoteConsoleExpect(connectionname, token, 
		connectiontouse->IsTargetHostLocalhost(), boost::chrono::seconds(15))));

	// Send a request that the target connects to us //
	shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(new RemoteConsoleOpenRequestDataTo(token)));


	connectiontouse->SendPacketToConnection(tmprequest, 4);
}
// ------------------------------------ //
void Leviathan::RemoteConsole::_OnNotifierDisconnected(ConnectionInfo* parenttoremove){
	// Close the corresponding console session //

	Logger::Get()->Info(L"RemoteConsole: detected connection closing, trying to close matching remote console session:");

	ObjectLock guard(*this);

	for(size_t i = 0; i < RemoteConsoleConnections.size(); i++){
		if(RemoteConsoleConnections[i]->GetConnection() == parenttoremove){
			// Close it //
			Logger::Get()->Info(L"\t> RemoteConsole: closed matching connection");
			RemoteConsoleConnections[i]->ResetConnection();
			RemoteConsoleConnections.erase(RemoteConsoleConnections.begin()+i);
			return;
		}
	}
	// Not found //
	Logger::Get()->Error(L"\t> RemoteConsole: didn't find matching connection, bug?");
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::HandleRemoteConsoleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	// First check if it should be handled by CanOpenNewConnection which handless all open connection packets //
	if(request->GetType() == NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE || request->GetType() == NETWORKREQUESTTYPE_OPENREMOTECONSOLETO){

		CanOpenNewConnection(connection, request);
		return;
	}

	ObjectLock guard(*this);

	// Handle normal RemoteConsole request //
	switch(request->GetType()){
	case NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE:
		{
			// Kill connection //
			GetRemoteConsoleSessionForConnection(connection, guard)->KillConnection();
			Logger::Get()->Info(L"RemoteConsole: closing connection due to close request");
		}
		break;
	default:
		DEBUG_BREAK;

	}
}

DLLEXPORT void Leviathan::RemoteConsole::HandleRemoteConsoleResponse(shared_ptr<NetworkResponse> response, ConnectionInfo* connection, 
	shared_ptr<NetworkRequest> potentialrequest)
{
	// We can detect close messages //
	switch(response->GetTypeOfResponse()){
	case NETWORKRESPONSETYPE_REMOTECONSOLECLOSED: case NETWORKRESPONSETYPE_REMOTECONSOLEOPENED:
		{
			// These shouldn't be received //
			Logger::Get()->Warning(L"RemoteConsole: HandleRemoteConsoleResponse: got a packet of type remote opened/remote closed, not expected to be received without"
				L" use requesting it");
		}
		break;
	default:
		Logger::Get()->Warning(L"RemoteConsole: HandleRemoteConsoleResponse: got a packet of unknown type, maybe this should not have been passed to RemoteConsole");
		DEBUG_BREAK;
	}
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RemoteConsole::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	throw std::exception();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::SetCloseIfNoRemoteConsole(bool state){
	CloseIfNoRemoteConsole = state;
}

DLLEXPORT RemoteConsoleSession* Leviathan::RemoteConsole::GetRemoteConsoleSessionForConnection(ConnectionInfo* connection, ObjectLock &guard){
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

DLLEXPORT ConnectionInfo* Leviathan::RemoteConsole::GetUnsafeConnectionForRemoteConsoleSession(const wstring &name){
	ObjectLock guard(*this);
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
Leviathan::RemoteConsole::RemoteConsoleExpect::RemoteConsoleExpect(const wstring &name, int token, bool onlylocalhost, const MillisecondDuration 
	&timeout) : ConnectionName(name), SessionToken(token), OnlyLocalhost(onlylocalhost), TimeoutTime(Misc::GetThreadSafeSteadyTimePoint()+timeout)
{

}
// ------------------ RemoteConsoleSession ------------------ //
Leviathan::RemoteConsoleSession::RemoteConsoleSession(const wstring &name, ConnectionInfo* connection, int token) : ConnectionName(name), 
	SessionToken(token), CorrespondingConnection(connection), TerminateSession(false)
{

}

Leviathan::RemoteConsoleSession::~RemoteConsoleSession(){
	// Send close request //
	if(CorrespondingConnection){

		shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE));
		
		CorrespondingConnection->SendPacketToConnection(tmprequest, 1);
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
