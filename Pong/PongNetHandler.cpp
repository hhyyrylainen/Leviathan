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
void Pong::PongNetHandler::HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection){
	// Try default handling //
	if(_HandleDefaultResponseOnly(message, connection))
		return;

	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}
// ------------------------------------ //




