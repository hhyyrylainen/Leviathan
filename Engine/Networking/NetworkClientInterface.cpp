#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKCLIENTINTERFACE
#include "NetworkClientInterface.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkClientInterface::NetworkClientInterface(){

}

DLLEXPORT Leviathan::NetworkClientInterface::~NetworkClientInterface(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkClientInterface::JoinServer(shared_ptr<ConnectionInfo> connectiontouse){
	return false;
}

DLLEXPORT void Leviathan::NetworkClientInterface::DisconnectFromServer(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkClientInterface::_HandleClientRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connectiontosendresult){
	DEBUG_BREAK;
	return false;
}

DLLEXPORT bool Leviathan::NetworkClientInterface::_HandleClientResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection, bool &dontmarkasreceived){
	DEBUG_BREAK;
	return false;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::UpdateClientStatus(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::_OnNotifierDisconnected(BaseNotifiableAll* parenttoremove){
	DEBUG_BREAK;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkClientInterface::_OnDisconnectFromServer(const wstring &reasonstring){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnStartConnectToServer(){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnFailedToConnectToServer(const wstring &reason){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnSuccessfullyConnectedToServer(){

}

DLLEXPORT void Leviathan::NetworkClientInterface::_OnNewConnectionStatusMessage(const wstring &message){

}
