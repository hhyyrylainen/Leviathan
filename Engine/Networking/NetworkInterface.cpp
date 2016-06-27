// ------------------------------------ //
#include "NetworkInterface.h"

#include "Exceptions.h"
#include "NetworkRequest.h"
#include "NetworkResponse.h"
#include "ConnectionInfo.h"
#include "RemoteConsole.h"
#include "Application/AppDefine.h"
#include "SyncedVariables.h"
#include "NetworkedInputHandler.h"
#include "../Utility/Convert.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
DLLEXPORT NetworkInterface::NetworkInterface(NETWORKED_TYPE type) :
    OurNetworkType(type)
{

}

DLLEXPORT NetworkInterface::~NetworkInterface(){

}
// ------------------------------------ //
DLLEXPORT void NetworkInterface::HandleRequestPacket(
    shared_ptr<NetworkRequest> request, Connection &connection) 
{
	// We can only try the default handle function //
	if(!_HandleDefaultRequest(request, connection)){
		// We couldn't handle it //

		throw InvalidArgument("could not handle request with default handler");
	}
}

DLLEXPORT bool NetworkInterface::PreHandleResponse(shared_ptr<NetworkResponse> response,
    std::shared_ptr<NetworkRequest> originalrequest, Connection &connection)
{
	return true;
}
// ------------------------------------ //
bool NetworkInterface::_HandleDefaultRequest(shared_ptr<NetworkRequest> request,
    Connection &connectiontosendresult)
{
	// Switch based on type //

	// See if it is a sync packet //
	if(SyncedVariables::Get()->HandleSyncRequests(request, connectiontosendresult))
		return true;

	switch(request->GetType()){
        case NETWORKREQUESTTYPE_IDENTIFICATION:
		{
			// Let's send our identification string //
			shared_ptr<NetworkResponse> tmpresponse(new NetworkResponse(request->GetExpectedResponseID(),
                    PACKET_TIMEOUT_STYLE_TIMEDMS, 500));

			// Fetch the data from the configuration object //
			string userreadable, gamename, gameversion;

			AppDef::GetDefault()->GetGameIdentificationData(userreadable, gamename, gameversion);

			// Set the right data //
			tmpresponse->GenerateIdentificationStringResponse(new NetworkResponseDataForIdentificationString(
                    userreadable, gamename, gameversion, LEVIATHAN_VERSION_ANSIS));
			connectiontosendresult->SendPacketToConnection(tmpresponse, 3);

			return true;
		}
        case NETWORKREQUESTTYPE_ECHO:
        {
            // Send an empty response back //
            std::shared_ptr<NetworkResponse> response(new NetworkResponse(request->GetExpectedResponseID(),
                    PACKET_TIMEOUT_STYLE_TIMEDMS, 1000));

            response->GenerateEmptyResponse();
            
            connectiontosendresult->SendPacketToConnection(response, 1);
            
            return true;
        }
        case NETWORKREQUESTTYPE_ACCESSREMOTECONSOLE: case NETWORKREQUESTTYPE_OPENREMOTECONSOLETO:
        case NETWORKREQUESTTYPE_CLOSEREMOTECONSOLE:
		{
			RemoteConsole::Get()->HandleRemoteConsoleRequestPacket(request, connectiontosendresult);

			return true;
		}
        case NETWORKREQUESTTYPE_CONNECTINPUT:
        {
            auto handler = NetworkedInputHandler::Get();
            if(handler)
                handler->HandleInputPacket(request, connectiontosendresult);

            return true;
        }
        default:
            return false;
	}

	// Unhandled //
	return false;
}
// ------------------------------------ //
bool NetworkInterface::_HandleDefaultResponseOnly(shared_ptr<NetworkResponse> message, Connection
    &connection, bool &dontmarkasreceived)
{

	// See if it is a sync packet //
	if(SyncedVariables::Get()->HandleResponseOnlySync(message, connection))
		return true;

	// Switch on type //
	switch(message->GetTypeOfResponse()){
        case NETWORKRESPONSETYPE_NONE:
        {
            // Empty packets without a matching request are just ignored, but marked as received
            return true;
        }
        case NETWORKRESPONSETYPE_KEEPALIVE:
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
			Logger::Get()->Info("NetworkInterface: dropping connection due to "
                "receiving a connection close packet (" +
                connection->GenerateFormatedAddressString() + ")");

			NetworkHandler::Get()->SafelyCloseConnectionTo(connection);
			return true;
		}
        case NETWORKRESPONSETYPE_REMOTECONSOLEOPENED: case NETWORKRESPONSETYPE_REMOTECONSOLECLOSED:
		{
			// Pass to remote console //
			RemoteConsole::Get()->HandleRemoteConsoleResponse(message, connection, NULL);
			return true;
		}
        case NETWORKRESPONSETYPE_CREATENETWORKEDINPUT: case NETWORKRESPONSETYPE_UPDATENETWORKEDINPUT:
        {
            auto handler = NetworkedInputHandler::Get();
            if(handler)
                handler->HandleInputPacket(message, connection);
            
            return true;
        }
            
        default:
            return false;
	}
	// Not handled //
	return false;
}
// ------------------------------------ //
DLLEXPORT bool NetworkInterface::CanConnectionTerminate(Connection &connection){
	// By default allow connections to close //
	return true;
}
// ------------------------------------ //
DLLEXPORT void NetworkInterface::TickIt(){
	return;
}
// ------------------------------------ //
DLLEXPORT void NetworkInterface::VerifyType(NETWORKED_TYPE type) const{

    LEVIATHAN_ASSERT(type == OurNetworkType, "NetworkInterface::VerifyType doesn't match");
}
// ------------------------------------ //
DLLEXPORT void NetworkInterface::SetOwner(NetworkHandler* owner){

    Owner = owner;
}
