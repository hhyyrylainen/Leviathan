#include "PongMasterServerIncludes.h"
// ------------------------------------ //
#ifndef PONGMASTERNETWORKING
#include "PongMasterNetworking.h"
#endif
using namespace Pong;
// ------------------------------------ //
Pong::PongMasterNetworking::PongMasterNetworking(){

}

Pong::PongMasterNetworking::~PongMasterNetworking(){

}
// ------------------------------------ //
void Pong::PongMasterNetworking::HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived){
	// Try default handling //
	if(_HandleDefaultResponseOnly(message, connection, dontmarkasreceived))
		return;

	// We couldn't handle it //
	Logger::Get()->Error(L"Couldn't handle a packet");
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //




