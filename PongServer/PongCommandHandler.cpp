#include "PongIncludes.h"
// ------------------------------------ //
#ifndef PONG_COMMANDHANDLER
#include "PongCommandHandler.h"
#endif
#include "Iterators/StringIterator.h"
#include "Gameplay/CommandHandler.h"
#include "PlayerSlot.h"
#include "PongServer.h"
#include "Networking/NetworkServerInterface.h"
using namespace Pong;
// ------------------------------------ //
Pong::PongCommandHandler::PongCommandHandler(PongServerNetworking* owner) : Owner(owner){

}

Pong::PongCommandHandler::~PongCommandHandler(){

}
// ------------------------------------ //
DLLEXPORT bool Pong::PongCommandHandler::CanHandleCommand(const string &cmd) const{
	// Just compare to our command strings //
	if(cmd == "join" || cmd == "open" || cmd == "kickslot" || cmd == "leave" || cmd == "ready" || cmd == "start"){

		return true;
	}

	return false;
}
// ------------------------------------ //
DLLEXPORT void Pong::PongCommandHandler::ExecuteCommand(const string &wholecommand, CommandSender* sender){

	StringIterator itr(new UTF8DataIterator(wholecommand));

	auto cmd = itr.GetNextCharacterSequence<string>(Leviathan::UNNORMALCHARACTER_TYPE_LOWCODES);

	// Perform something based on the command //
	if(*cmd == "join"){

		// Get the target slot and target split //
		auto targetslot = itr.GetNextNumber<string>(Leviathan::DECIMALSEPARATORTYPE_NONE);

		auto targetsplit = itr.GetNextNumber<string>(Leviathan::DECIMALSEPARATORTYPE_NONE);

		// The variables for the join //
		int slotnumber;
		bool split;


		// Process them //
		if(!targetsplit || targetsplit->empty()){

			split = false;
		} else {

			int tmpvar = Convert::StringTo<int>(*targetsplit);

			if(!tmpvar){

				split = false;
			} else {

				split = true;
			}
		}


		// Then the slot //
		if(!targetslot || targetslot->empty()){

			slotnumber = -1;
		} else {

			int tmpvar = Convert::StringTo<int>(*targetslot);

			slotnumber = tmpvar;
		}


		// Check are the values right //
		if(slotnumber < 0 || slotnumber > 3){

			sender->SendPrivateMessage("invalid slot number");
			return;
		}

		PlayerList* slots = PongServer::Get()->GetPlayers();

		// Check is the slot empty //
		auto chosenslot = slots->GetSlot(slotnumber);

		if(split)
			chosenslot = chosenslot->GetSplit();

		if(!chosenslot){

			sender->SendPrivateMessage("The chosen slot/sub-slot is not open/created.");
			return;
		}

		if(chosenslot->GetPlayerType() != PLAYERTYPE_EMPTY){

			// Somebody has taken it //
			sender->SendPrivateMessage("that slot is already taken");
			return;
		}

		// Add this player to the slot //
		chosenslot->SlotJoinPlayer(dynamic_cast<Leviathan::ConnectedPlayer*>(sender));

		// Create a controller id for this player //
		int newid = PongServer::Get()->GetServerNetworkInterface()->GetNetworkedInput()->GetNextInputIDNumberOnServer();

		chosenslot->SetNetworkedInputID(newid);

		Logger::Get()->Info(L"Player joined slot "+Convert::ToWstring(slotnumber)+L", split "+Convert::ToWstring<int>(split)+L" (networked control: "+
			Convert::ToWstring(newid)+L") named "
			+Convert::Utf8ToUtf16(sender->GetNickname()));

		slots->NotifyUpdatedValue();
	} else if(*cmd == "open"){

		// Get the target slot and target split //
		auto targetslot = itr.GetNextNumber<string>(Leviathan::DECIMALSEPARATORTYPE_NONE);

		auto targetsplit = itr.GetNextNumber<string>(Leviathan::DECIMALSEPARATORTYPE_NONE);

		// The variables for the join //
		int slotnumber;
		bool split;


		// Process them //
		if(!targetsplit || targetsplit->empty()){

			split = false;
		} else {

			int tmpvar = Convert::StringTo<int>(*targetsplit);

			if(!tmpvar){

				split = false;
			} else {

				split = true;
			}
		}


		// Then the slot //
		if(!targetslot || targetslot->empty()){

			slotnumber = -1;
		} else {

			int tmpvar = Convert::StringTo<int>(*targetslot);

			slotnumber = tmpvar;
		}


		// Check are the values right //
		if(slotnumber < 0 || slotnumber > 3){

			sender->SendPrivateMessage("invalid slot number");
			return;
		}

		PlayerList* slots = PongServer::Get()->GetPlayers();

		// Check is the slot empty //
		auto chosenslot = slots->GetSlot(slotnumber);

		if(split){

			auto splitslot = chosenslot->GetSplit();

			if(!splitslot){

				// Add a new sub-slot //
				chosenslot->AddEmptySubSlot();

				// And set it as open //
				chosenslot->GetSplit()->SetPlayer(PLAYERTYPE_EMPTY, 0);

				Logger::Get()->Info(L"New split slot opened, "+Convert::ToWstring(slotnumber));
				slots->NotifyUpdatedValue();
				return;
			}

			chosenslot = splitslot;
		}

		if(chosenslot->GetPlayerType() != PLAYERTYPE_CLOSED){

			// Somebody has opened it //
			sender->SendPrivateMessage("that slot is already open");
			return;
		}

		// Open the slot //
		chosenslot->SetPlayer(PLAYERTYPE_EMPTY, 0);

		// Reset some settings //
		chosenslot->SetControls(PLAYERCONTROLS_NONE, 0);
		chosenslot->SetColour(Float4::GetColourWhite());

		Logger::Get()->Info(L"Slot opened, "+Convert::ToWstring(slotnumber));

		slots->NotifyUpdatedValue();
	}


}
// ------------------------------------ //



