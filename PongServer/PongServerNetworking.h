#ifndef PONGSERVERNETWORKING
#define PONGSERVERNETWORKING
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Networking/NetworkInterface.h"



namespace Pong{

	class PongServerNetworking : public Leviathan::NetworkInterface{
	public:
		PongServerNetworking();
		virtual ~PongServerNetworking();

		virtual void HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection);

	protected:

	};

}
#endif