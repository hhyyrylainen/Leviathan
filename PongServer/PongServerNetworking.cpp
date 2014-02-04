#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONGSERVERNETWORKING
#include "PongServerNetworking.h"
#endif
#include "Networking/NetworkRequest.h"
#include "Networking/ConnectionInfo.h"
#include "Networking/NetworkResponse.h"
using namespace Pong;
// ------------------------------------ //
Pong::PongServerNetworking::PongServerNetworking() : NetworkServerInterface(8, L"Local pong game", Leviathan::NETWORKRESPONSE_SERVERJOINRESTRICT_NONE){

}

Pong::PongServerNetworking::~PongServerNetworking(){

}
// ------------------------------------ //
void Pong::PongServerNetworking::HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived){
	// Try default handling //
	if(_HandleDefaultResponseOnly(message, connection, dontmarkasreceived))
		return;
	if(_HandleServerResponseOnly(message, connection, dontmarkasreceived))
		return;


	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}

void Pong::PongServerNetworking::HandleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	// Try default handling //
	if(_HandleDefaultRequest(request, connection))
		return;
	// Try server handling //
	if(_HandleServerRequest(request, connection))
		return;

	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}

void Pong::PongServerNetworking::TickIt(){
	// Not a client, nothing to do //
}

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //




