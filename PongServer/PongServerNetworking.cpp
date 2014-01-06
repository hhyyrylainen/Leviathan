#include "PongServerIncludes.h"
// ------------------------------------ //
#ifndef PONGSERVERNETWORKING
#include "PongServerNetworking.h"
#endif
using namespace Pong;
// ------------------------------------ //
Pong::PongServerNetworking::PongServerNetworking(){

}

Pong::PongServerNetworking::~PongServerNetworking(){

}
// ------------------------------------ //
void Pong::PongServerNetworking::HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection){
	// Try default handling //
	if(_HandleDefaultResponseOnly(message, connection))
		return;

	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //




