// ------------------------------------ //
#include "PongMasterNetworking.h"

#include "PongMasterServerIncludes.h"
using namespace Pong;
using namespace std;
// ------------------------------------ //
Pong::PongMasterNetworking::PongMasterNetworking(){

}

Pong::PongMasterNetworking::~PongMasterNetworking(){

}
// ------------------------------------ //
void Pong::PongMasterNetworking::HandleResponseOnlyPacket(
    shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection,
    bool &dontmarkasreceived)
{
	// Try default handling //
	if(_HandleDefaultResponseOnly(message, connection, dontmarkasreceived))
		return;

	// We couldn't handle it //
	Logger::Get()->Error("Couldn't handle a packet");
}
// ------------------------------------ //
void Pong::PongMasterNetworking::CloseDown(){
	
	Logger::Get()->Info("Should probably put stuff here");
}
// ------------------------------------ //

// ------------------------------------ //




