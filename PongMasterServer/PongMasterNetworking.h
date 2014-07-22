#ifndef PONGMASTERNETWORKING
#define PONGMASTERNETWORKING
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Networking/NetworkInterface.h"



namespace Pong{

	class PongMasterNetworking : public Leviathan::NetworkInterface{
	public:
		PongMasterNetworking();
		virtual ~PongMasterNetworking();

		virtual void HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, 
			bool &dontmarkasreceived);
		
		//! \todo Put actual stuff here
		virtual void CloseDown();
		
		
	protected:

	};

}
#endif