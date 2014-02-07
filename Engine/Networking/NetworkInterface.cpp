#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_NETWORKINTERFACE
#include "NetworkInterface.h"
#endif
#include "Exceptions/ExceptionInvalidArgument.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
#include "ConnectionInfo.h"
#include "RemoteConsole.h"
#include "Application/AppDefine.h"
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

			// Fetch the data from the configuration object //
			wstring userreadable, gamename, gameversion;

			AppDef::GetDefault()->GetGameIdentificationData(userreadable, gamename, gameversion);

			// Set the right data //
			tmpresponse->GenerateIdentificationStringResponse(new NetworkResponseDataForIdentificationString(userreadable, gamename, gameversion, 
				LEVIATHAN_VERSIONS));
			connectiontosendresult->SendPacketToConnection(tmpresponse, 3);

			return true;
		}
	case NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE: case NETWORKREQUESTTYPE_OPENREMOTECONSOLETO: case NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE:
		{
			RemoteConsole::Get()->HandleRemoteConsoleRequestPacket(request, connectiontosendresult);

			return true;
		}
	}

	// Unhandled //
	return false;
}
// ------------------------------------ //
bool Leviathan::NetworkInterface::_HandleDefaultResponseOnly(shared_ptr<NetworkResponse> message, ConnectionInfo* connection, bool &dontmarkasreceived){
	// Switch on type //
	switch(message->GetTypeOfResponse()){
	case NETWORKRESPONSETYPE_KEEPALIVE: case NETWORKRESPONSETYPE_NONE:
		{
			// Requires no handling //
			// Also this should not be reported as received //
			dontmarkasreceived = true;

			// Actually might want to respond with a keepalive packet //
			connection->CheckKeepAliveSend();
			return true;
		}
	case NETWORKRESPONSETYPE_CLOSECONNECTION:
		{
			// This connection should be closed //
			Logger::Get()->Info(L"NetworkInterface: dropping connection due to receiving a connection close packet ("+
				connection->GenerateFormatedAddressString()+L")");

			NetworkHandler::Get()->SafelyCloseConnectionTo(connection);
			return true;
		}
	case NETWORKRESPONSETYPE_REMOTECONSOLEOPENED: case NETWORKRESPONSETYPE_REMOTECONSOLECLOSED:
		{
			// Pass to remote console //
			RemoteConsole::Get()->HandleRemoteConsoleResponse(message, connection, NULL);
			return true;
		}

	}
	// Not handled //
	return false;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::NetworkInterface::CanConnectionTerminate(ConnectionInfo* connection){
	// By default allow connections to close //
	return true;
}
// ------------------------------------ //
void Leviathan::NetworkInterface::_SetNetworkType(NETWORKED_TYPE ntype){
	OurNetworkType = ntype;
}

DLLEXPORT void Leviathan::NetworkInterface::TickIt(){
	return;
}
