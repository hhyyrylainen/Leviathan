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
bool Pong::PongCommandHandler::CanHandleCommand(const string &cmd) const{
    // Just compare to our command strings //
    if(cmd == "join" || cmd == "open" || cmd == "kickslot" || cmd == "leave" || cmd == "ready"
        || cmd == "start" || cmd == "close" || cmd == "controls" || cmd == "colour")
    {
        return true;
    }

    return false;
}
// ------------------------------------ //
void Pong::PongCommandHandler::ExecuteCommand(const string &wholecommand,
    CommandSender* sender)
{
    StringIterator itr(std::make_unique<UTF8DataIterator>(wholecommand));

    auto cmd = itr.GetNextCharacterSequence<string>(
        Leviathan::UNNORMALCHARACTER_TYPE_LOWCODES);

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

        PlayerList* slots = BasePongParts::Get()->GetPlayers();

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

        {
            Lock lock(PlayerIDMutex);
            
            // Add this player to the slot //
            chosenslot->SlotJoinPlayer(dynamic_cast<Leviathan::ConnectedPlayer*>(sender), ++PlayerUniqueCounter);
            
        }

        // Create a controller id for this player //
        DEBUG_BREAK
        //int newid = PongServer::Get()->GetServerNetworkInterface()->GetNetworkedInput()->GetNextInputIDNumberOnServer();

        //chosenslot->SetNetworkedInputID(newid);

        //// \todo Move this to some proper place
        //chosenslot->SetControls(PLAYERCONTROLS_ARROWS, 0);


        //Logger::Get()->Info("Player joined slot "+Convert::ToString(slotnumber)+", split "+
        //    Convert::ToString<int>(split)+" (networked control: "+Convert::ToString(newid)+
        //    ") named "+sender->GetNickname());

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

        PlayerList* slots = BasePongParts::Get()->GetPlayers();

        // Check is the slot empty //
        auto chosenslot = slots->GetSlot(slotnumber);

        if(split){

            auto splitslot = chosenslot->GetSplit();

            if(!splitslot){

                // Add a new sub-slot //
                chosenslot->AddEmptySubSlot();

                // And set it as open //
                chosenslot->GetSplit()->SetPlayer(PLAYERTYPE_EMPTY, 0);

                Logger::Get()->Info("New split slot opened, "+Convert::ToString(slotnumber));
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

        slots->NotifyUpdatedValue();

    } else if(*cmd == "start"){

        ThreadingManager::Get()->QueueTask(new QueuedTask(std::bind<void>([]() -> void
            {

                Logger::Get()->Info("TODO: check permissions");


                Logger::Get()->Info("TODO: check can a match actually begin");

                // Start the match //
                PongServer::Get()->OnStartPreMatch();

            })));
        
    } else if(*cmd == "controls"){

        // Get the target parameters //
        auto targetslot = itr.GetNextNumber<string>(Leviathan::DECIMALSEPARATORTYPE_NONE);

        auto targetsplit = itr.GetNextNumber<string>(Leviathan::DECIMALSEPARATORTYPE_NONE);

        auto targetcontrols = itr.GetNextNumber<string>(Leviathan::DECIMALSEPARATORTYPE_NONE);

        auto targetnumber = itr.GetNextNumber<string>(Leviathan::DECIMALSEPARATORTYPE_NONE);

        bool split;

        // Fail if invalid format //
        if(!targetsplit || !targetslot || !targetcontrols || !targetnumber)
            return;

        int slotnumber = Convert::StringTo<int>(*targetslot);

        int controls = Convert::StringTo<int>(*targetcontrols);

        int number = Convert::StringTo<int>(*targetnumber);

        int tmpvar = Convert::StringTo<int>(*targetsplit);

        if(!tmpvar){

            split = false;
        } else {

            split = true;
        }

        if(slotnumber < 0 || slotnumber > 3){

            sender->SendPrivateMessage("invalid slot number");
            return;
        }
        
        // Find the target slot //

        PlayerList* slots = BasePongParts::Get()->GetPlayers();

        // Check is the slot empty //
        auto chosenslot = slots->GetSlot(slotnumber);

        if(split){

            chosenslot = chosenslot->GetSplit();
        }

        Logger::Get()->Info("TODO: check whether the player changing controls is the player in the slot");

        if(!chosenslot->IsSlotActive())
            return;

        Logger::Get()->Info("Pong: setting slot "+Convert::ToString(slotnumber)+" split "+
            Convert::ToString(split)+"controls to "+Convert::ToString(controls)+
            ", "+Convert::ToString(number));
        
        chosenslot->SetControls(static_cast<PLAYERCONTROLS>(controls), number);

        slots->NotifyUpdatedValue();
        
    } else {

        Logger::Get()->Warning("Didn't recognize command \""+*cmd+"\"");
    }
}
// ------------------------------------ //



