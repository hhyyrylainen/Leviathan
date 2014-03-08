#ifndef PONGSERVERNETWORKING
#define PONGSERVERNETWORKING
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Networking/NetworkInterface.h"
#include "Networking/NetworkServerInterface.h"
#include "PongPackets.h"



namespace Pong{

	class PongServerNetworking : public Leviathan::NetworkInterface, public Leviathan::NetworkServerInterface{
	public:
		PongServerNetworking();
		virtual ~PongServerNetworking();

		virtual void HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived);
		virtual void HandleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection);

		virtual void TickIt();

	protected:


		PONG_JOINGAMERESPONSE_TYPE ServerStatusIs;
	};

}
#endif