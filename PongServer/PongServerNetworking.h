#pragma once
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
#include "Networking/NetworkInterface.h"
#include "Networking/NetworkServerInterface.h"
#include "PongPackets.h"
#include "PongCommandHandler.h"
#include <memory>


namespace Pong{

    using namespace std;

	class PongServerNetworking : public Leviathan::NetworkInterface,
                                   public Leviathan::NetworkServerInterface
    {
	public:
		PongServerNetworking();
		virtual ~PongServerNetworking();

		virtual void HandleResponseOnlyPacket(shared_ptr<Leviathan::NetworkResponse> message,
            Leviathan::ConnectionInfo* connection, bool &dontmarkasreceived);
		virtual void HandleRequestPacket(shared_ptr<NetworkRequest> request,
            ConnectionInfo* connection);

		virtual void TickIt();

		virtual void CloseDown();


		//! Sets the current state and notifies clients
		void SetStatus(PONG_JOINGAMERESPONSE_TYPE status);

		//! Makes sure that all the players are synced with a world
		void VerifySyncWorldForPlayers(Leviathan::GameWorld* world);

	protected:

		virtual void RegisterCustomCommandHandlers(CommandHandler* addhere);

		//! \brief Removes the player from the game
		virtual void _OnPlayerDisconnect(Leviathan::ConnectedPlayer* newplayer);


		PONG_JOINGAMERESPONSE_TYPE ServerStatusIs;
	};

}

