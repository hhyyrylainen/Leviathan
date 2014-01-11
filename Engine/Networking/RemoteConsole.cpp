#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_REMOTECONSOLE
#include "RemoteConsole.h"
#endif
#include "ConnectionInfo.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::RemoteConsole::RemoteConsole(){
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
	auto timenow = boost::chrono::steady_clock::now();

	for(size_t i = 0; i < AwaitingConnections.size(); i++){
		if(AwaitingConnections[i]->TimeoutTime < timenow){
			// Time it out //
			Logger::Get()->Warning(L"RemoteConsole: Remote console wait connection timed out, token "+Convert::ToWstring(AwaitingConnections[i]->SessionToken));
			AwaitingConnections.erase(AwaitingConnections.begin()+i);
			i--;
			continue;
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

	if(!tmpdata){
		// Invalid package data/type //
		return false;
	}

	// Look for a matching awaiting connection //
	for(size_t i = 0; i < AwaitingConnections.size(); i++){
		if(AwaitingConnections[i]->OnlyLocalhost && local || !AwaitingConnections[i]->OnlyLocalhost){
			if(AwaitingConnections[i]->SessionToken == tmpdata->SessionToken){
				// Match found //
				Logger::Get()->Info(L"RemoteConsole: matching connection request got!");

				// Add to real connections //
				RemoteConsoleConnections.push_back(shared_ptr<RemoteConsoleSession>(new RemoteConsoleSession(AwaitingConnections[i]->ConnectionName,
					connection, AwaitingConnections[i]->SessionToken)));

				// Create a open request //
				shared_ptr<NetworkRequest> tmprequest(new NetworkRequest(new RemoteConsoleAccessRequestData(AwaitingConnections[i]->SessionToken)));

				// Send initial packet //
				connection->SendPacketToConnection(tmprequest, 3);
				WaitingRequests.push_back(tmprequest);

				AwaitingConnections.erase(AwaitingConnections.begin()+i);
				break;
			}
		}
	}

	// Didn't find a match //
	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::OfferConnectionTo(const sf::IpAddress &targetip, USHORT port, const wstring &connectionname){
	DEBUG_BREAK;
}
// ------------------------------------ //
void Leviathan::RemoteConsole::_OnNotifierDisconnected(BaseNotifier* parenttoremove){
	DEBUG_BREAK;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::RemoteConsole::HandleRemoteConsoleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){

}

DLLEXPORT void Leviathan::RemoteConsole::HandleRemoteConsoleResponse(shared_ptr<NetworkResponse> response, ConnectionInfo* connection, 
	shared_ptr<NetworkRequest> potentialrequest)
{

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::RemoteConsole::SendCustomMessage(int entitycustommessagetype, void* dataptr){
	throw std::exception();
}
// ------------------ RemoteConsoleExpect ------------------ //
Leviathan::RemoteConsole::RemoteConsoleExpect::RemoteConsoleExpect(const wstring &name, int token, bool onlylocalhost, const MillisecondDuration 
	&timeout) : ConnectionName(name), SessionToken(token), OnlyLocalhost(onlylocalhost), TimeoutTime(boost::chrono::steady_clock::now()+timeout)
{

}
// ------------------ RemoteConsoleSession ------------------ //
Leviathan::RemoteConsoleSession::RemoteConsoleSession(const wstring &name, ConnectionInfo* connection, int token) : ConnectionName(name), 
	SessionToken(token), CorrespondingConnection(connection)
{

}
