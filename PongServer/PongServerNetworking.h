#ifndef PONGSERVERNETWORKING
#define PONGSERVERNETWORKING
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Networking/NetworkInterface.h"
#include "Networking/NetworkServerInterface.h"
#include "PongPackets.h"
#include "PongCommandHandler.h"


namespace Pong{

	class PongServerNetworking : public Leviathan::NetworkInterface, public Leviathan::NetworkServerInterface{
	public:
		PongServerNetworking();
		virtual ~PongServerNetworking();

		virtual void HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived);
		virtual void HandleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection);

		virtual void TickIt();

		virtual void CloseDown();



	protected:

		virtual void RegisterCustomCommandHandlers(CommandHandler* addhere);


		PONG_JOINGAMERESPONSE_TYPE ServerStatusIs;
	};

}
#endif