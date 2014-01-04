#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKINTERFACE
#include "NetworkInterface.h"
#endif
#include "Exceptions\ExceptionInvalidArgument.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
#include "ConnectionInfo.h"
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::NetworkInterface::NetworkInterface(){

}

DLLEXPORT Leviathan::NetworkInterface::~NetworkInterface(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::NetworkInterface::HandleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection) THROWS{
	// We can only try the default handle function //
	if(!_HandleDefaultRequest(request, connection)){
		// We couldn't handle it //

		throw ExceptionInvalidArgument(L"could not handle request with default handler", 0, __WFUNCTION__, L"request", L"unknown type");
	}
}

DLLEXPORT bool Leviathan::NetworkInterface::PreHandleResponse(shared_ptr<NetworkResponse> response, shared_ptr<NetworkRequest> originalrequest, ConnectionInfo* connection){
	return true;
}
// ------------------------------------ //
bool Leviathan::NetworkInterface::_HandleDefaultRequest(shared_ptr<NetworkRequest> request, ConnectionInfo* connectiontosendresult){
	// Switch based on type //

	switch(request->GetType()){
	case NETWORKREQUESTTYPE_IDENTIFICATION:
		{
			// Let's send our identification string //
			shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(), PACKAGE_TIMEOUT_STYLE_TIMEDMS, 500));
			// Set the right data //
			tmpresponse->GenerateIdentificationStringResponse(new NetworkResponseDataForIdentificationString(L"TODO: this", L"TODO: fetch pong", 
				L"TODO: get version", LEVIATHAN_VERSIONS)); 
			connectiontosendresult->SendPacketToConnection(tmpresponse, 10);

			return true;
		}


	}

	// Unhandled //
	return false;
}
// ------------------------------------ //
bool Leviathan::NetworkInterface::_HandleDefaultResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection){
	// Switch on type //
	switch(message->GetTypeOfResponse()){
	case NETWORKRESPONSETYPE_KEEPALIVE: case NETWORKRESPONSETYPE_NONE:
		{
			// Requires no handling //
			return true;
		}


	}
	// Not handled //
	return false;
}



