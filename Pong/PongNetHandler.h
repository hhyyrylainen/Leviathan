#ifndef PONGNETHANDLER
#define PONGNETHANDLER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Networking/NetworkInterface.h"



namespace Pong{

	class PongNetHandler : public Leviathan::NetworkInterface{
	public:
		PongNetHandler();
		virtual ~PongNetHandler();

		virtual void HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived);


	protected:

	};

}
#endif