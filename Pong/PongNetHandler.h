#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Networking/NetworkInterface.h"
#include "Networking/NetworkClientInterface.h"
#include "PongPackets.h"


namespace Pong{

	//! \brief Pong specific NetworkClientInterface
	class PongNetHandler : public Leviathan::NetworkClientInterface{
	public:
		PongNetHandler();
		virtual ~PongNetHandler();

		//! \brief Joins the lobby or the match when the connection is confirmed
		virtual void _OnProperlyConnected();

	protected:


		//! \brief Used to fire GenericEvents to update GUI status
		virtual void _OnNewConnectionStatusMessage(const string &message);

		//! \brief This detects when the server kicks us and displays the reason
		virtual void _OnDisconnectFromServer(const string &reasonstring, bool donebyus);
	};

}

