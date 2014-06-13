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


		PongServerNetworking* Owner;
	};

}
#endif