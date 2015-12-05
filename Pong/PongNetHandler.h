#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Networking/NetworkInterface.h"
#include "Networking/NetworkClientInterface.h"
#include "PongPackets.h"


namespace Pong{

	//! \brief Pong specific NetworkClientInterface
	class PongNetHandler : public Leviathan::NetworkInterface, public Leviathan::NetworkClientInterface{
	public:
		PongNetHandler();
		virtual ~PongNetHandler();

		virtual void HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message, Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived);
		virtual void HandleRequestPacket(shared_ptr<NetworkRequest> request, ConnectionInfo* connection);

		virtual void TickIt();

		//! \brief Joins the lobby or the match when the connection is confirmed
		virtual void _OnStartApplicationConnect();


		virtual void CloseDown();

	protected:


		//! \brief Used to fire GenericEvents to update GUI status
		virtual void _OnNewConnectionStatusMessage(const string &message);

		//! \brief This detects when the server kicks us and displays the reason
		virtual void _OnDisconnectFromServer(const string &reasonstring, bool donebyus);


		bool OnAServer;
		PONG_JOINGAMERESPONSE_TYPE ServerStatusIs;
	};

}

