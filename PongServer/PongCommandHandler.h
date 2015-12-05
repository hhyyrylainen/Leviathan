#pragma once
// ------------------------------------ //
#include "PongIncludes.h"
// ------------------------------------ //
#include "Gameplay/CustomCommandHandler.h"
#include "Common/ThreadSafe.h"
#include <string>

namespace Pong{

	class PongServerNetworking;

    using namespace std;

	//! \brief Pong's command handler
	//! \see Leviathan::CustomCommandHandler
	class PongCommandHandler : public Leviathan::CustomCommandHandler{
	public:
		PongCommandHandler(PongServerNetworking* owner);
		~PongCommandHandler();


		DLLEXPORT virtual bool CanHandleCommand(const string &cmd) const;

		DLLEXPORT virtual void ExecuteCommand(const string &wholecommand, CommandSender* sender);

	private:

        //! Number for opened players (this has to be unique between all slots)
        int PlayerUniqueCounter;

        //! Mutex for PlayerUniqueCounter
        Mutex PlayerIDMutex;
        
		PongServerNetworking* Owner;
	};

}

