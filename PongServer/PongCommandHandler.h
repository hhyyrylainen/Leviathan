#pragma once
#ifndef PONG_COMMANDHANDLER
#define PONG_COMMANDHANDLER
// ------------------------------------ //
#ifndef PONGINCLUDES
#include "PongIncludes.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Gameplay/CustomCommandHandler.h"
#include "boost/thread/mutex.hpp"

namespace Pong{

	class PongServerNetworking;

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
        boost::mutex PlayerIDMutex;
        
		PongServerNetworking* Owner;
	};

}
#endif
