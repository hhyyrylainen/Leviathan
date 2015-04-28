#pragma once
// ------------------------------------ //
#include "Networking/NetworkInterface.h"


namespace Pong{

	class PongMasterNetworking : public Leviathan::NetworkInterface{
	public:
		PongMasterNetworking();
		virtual ~PongMasterNetworking();

		virtual void HandleResponseOnlyPacket(std::shared_ptr<Leviathan::NetworkResponse> message,
            Leviathan::ConnectionInfo* connection,  bool &dontmarkasreceived);
		
		//! \todo Put actual stuff here
		virtual void CloseDown();
		
		
	protected:

	};

}

