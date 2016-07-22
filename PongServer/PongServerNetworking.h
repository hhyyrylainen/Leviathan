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

    class PongServerNetworking : public Leviathan::NetworkServerInterface
    {
    public:
        PongServerNetworking();
        virtual ~PongServerNetworking();

        //! Sets the current state and notifies clients
        void SetStatus(PONG_JOINGAMERESPONSE_TYPE status);

    protected:

        virtual void RegisterCustomCommandHandlers(CommandHandler* addhere);

        //! \brief Removes the player from the game
        virtual void _OnPlayerDisconnect(Leviathan::ConnectedPlayer* newplayer);


        PONG_JOINGAMERESPONSE_TYPE ServerStatusIs;
    };

}

