#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONGNETHANDLER
#include "PongNetHandler.h"
#endif
using namespace Pong;
// ------------------------------------ //
Pong::PongNetHandler::PongNetHandler(){

}

Pong::PongNetHandler::~PongNetHandler(){

}
// ------------------------------------ //
void Pong::PongNetHandler::HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived){
	// Try default handling //
	if(_HandleDefaultResponseOnly(message, connection, dontmarkasreceived))
		return;
	// Try client handling //
	if(_HandleClientResponseOnly(message, connection, dontmarkasreceived))
		return;

	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}

void Pong::PongNetHandler::HandleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection){
	// Try default //
	if(_HandleDefaultRequest(request, connection))
		return;
	// Try client handling //
	if(_HandleClientRequest(request, connection))
		return;

	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}

DLLEXPORT void Pong::PongNetHandler::TickIt(){
	UpdateClientStatus();
}
// ------------------------------------ //




