#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKHANDLER
#include "NetworkHandler.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NetworkClient* clientside) : IsClient(clientside), IsServer(NULL){

}

DLLEXPORT Leviathan::NetworkHandler::NetworkHandler(NetworkServer* serverside) : IsServer(serverside), IsClient(NULL){

}

DLLEXPORT Leviathan::NetworkHandler::~NetworkHandler(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkHandler::Init(const MasterServerInformation &info){
	// Query master server //
	QueryMasterServer(info);
}

DLLEXPORT void Leviathan::NetworkHandler::Release(){
	// Close all connections //
	if(IsClient){

		IsClient->Release();
		IsClient = NULL;
	}
	if(IsServer){

		IsServer->Release();
		IsServer = NULL;
	}
}
// ------------------------------------ //
DLLEXPORT shared_ptr<DelayedResult> Leviathan::NetworkHandler::QueryMasterServer(const MasterServerInformation &info){

}
// ------------------------------------ //

// ------------------------------------ //



