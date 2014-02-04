#ifndef PONGNETHANDLER
#define PONGNETHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Networking/NetworkInterface.h"
#include "Networking/NetworkClientInterface.h"



namespace Pong{

	class PongNetHandler : public Leviathan::NetworkInterface, public Leviathan::NetworkClientInterface{
	public:
		PongNetHandler();
		virtual ~PongNetHandler();

		virtual void HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived);
		virtual void HandleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection);

		virtual void TickIt();

	protected:

	};

}
#endif