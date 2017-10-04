// ------------------------------------ //
#include "CommandHandler.h"

#include "Iterators/StringIterator.h"
#include "Threading/ThreadingManager.h"
using namespace Leviathan;
// ------------------------------------ //

//! \brief Runs the thing
//! \param sender The sender to pass to the handler, this will be verified to be
//! still valid before usage
void RunCustomHandler(std::shared_ptr<CustomCommandHandler> handler,
    std::shared_ptr<std::string> command, CommandSender* sender)
{
	Lock cmdlock;

	auto cmdhandler = CommandHandler::Get();

	// Cannot do anything if the handler no longer exist //
	if(!cmdhandler)
		return;

	// Check that the sender is still valid //
	Lock senderlock;
	if(!cmdhandler->IsSenderStillValid(sender)){

		// it isn't there anymore //
		return;
	}

	handler->ExecuteCommand(*command, sender);

	// The sender is now no longer required //
	cmdhandler->SenderNoLongerRequired(sender);
}


// ------------------ CommandHandler ------------------ //
DLLEXPORT Leviathan::CommandHandler::CommandHandler(NetworkServerInterface* owneraccess) :
    Owner(owneraccess)
{
	Staticaccess = this;
}

DLLEXPORT Leviathan::CommandHandler::~CommandHandler(){
    
    Staticaccess = NULL;

	CustomHandlers.clear();

	// All the pointers have to be removed before deleting this //
	_LetGoOfAll();
}

CommandHandler* Leviathan::CommandHandler::Staticaccess;

DLLEXPORT CommandHandler* Leviathan::CommandHandler::Get(){
    
	return Staticaccess;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::QueueCommand(const std::string &command,
    CommandSender* issuer)
{
	// Get the first word //
	StringIterator itr(std::make_unique<UTF8DataIterator>(command));

	// Only skip a single / if there is one //
	if(itr.GetCharacter() == '/')
		itr.MoveToNext();

	// Get the first word //
	auto firstword = itr.GetNextCharacterSequence<std::string>(
        UNNORMALCHARACTER_TYPE_LOWCODES);

	if(!firstword || firstword->empty())
		return;



	// Find a command handler that provides handling for this command //

	// First check the default commands //
	if(IsThisDefaultCommand(*firstword)){

		// Queue a default command execution //
		DEBUG_BREAK;

		// Default will also need this //
		_AddSender(issuer);
		return;
	}

	// Loop through and check the custom command handlers //
	auto end = CustomHandlers.end();
	for(auto iter = CustomHandlers.begin(); iter != end; ++iter){

		if((*iter)->CanHandleCommand(*firstword)){
			// Queue the command handler //
			ThreadingManager::Get()->QueueTask(new QueuedTask(std::bind(&RunCustomHandler,
                        *iter, std::make_shared<std::string>(command), issuer)));


			// And take good care of the object while the command handler is waiting //
			_AddSender(issuer);
			return;
		}
	}

	// Couldn't find a handler for command //
	issuer->SendPrivateMessage("Unknown command \""+*firstword+"\"");
}

DLLEXPORT void Leviathan::CommandHandler::RemoveMe(CommandSender* object){

	// Remove from the vector //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){

		if(*iter == object){
			// Remove the match //
			SendersInUse.erase(iter);
			return;
		}
	}
}

DLLEXPORT bool Leviathan::CommandHandler::IsSenderStillValid(CommandSender* checkthis){
    
	// Check is it still in the list //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){
		if(*iter == checkthis){
			// It is still there //
			return true;
		}
	}

	// Not found //
	return false;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::UpdateStatus(){


}
// ------------------------------------ //
void Leviathan::CommandHandler::_LetGoOfAll(){

	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){

		(*iter)->EndOwnership(this);
	}

	// Clear them all at once //
	SendersInUse.clear();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::SenderNoLongerRequired(CommandSender* checkthis){

	// Remove from the vector //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){

		if(*iter == checkthis){
			// Notify it //
			(*iter)->EndOwnership(this);

			// Remove the match //
			SendersInUse.erase(iter);
			return;
		}
	}
}

void Leviathan::CommandHandler::_AddSender(CommandSender* object){

	// Notify the object //
	object->StartOwnership(this);

	// Add to the list //
	SendersInUse.push_back(object);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::CommandHandler::RegisterCustomCommandHandler(
    std::shared_ptr<CustomCommandHandler> handler)
{
	// Might be unnecessary to check this, but it's a way to return false sometimes //
	if(!handler)
		return false;

	CustomHandlers.push_back(handler);
	return true;
}

DLLEXPORT bool Leviathan::CommandHandler::IsThisDefaultCommand(const std::string &firstword)
    const
{
	// Didn't match any of the strings, cannot be //
	return false;
}
// ------------------ CommandSender ------------------ //
DLLEXPORT void Leviathan::CommandSender::StartOwnership(CommandHandler* commander){

	// Just add to the list //
	CommandHandlersToNotify.push_back(commander);
}

DLLEXPORT void Leviathan::CommandSender::EndOwnership(CommandHandler* which){

	// Find the right one and remove it //
	auto end = CommandHandlersToNotify.end();
	for(auto iter = CommandHandlersToNotify.begin(); iter != end; ++iter){

		if(*iter == which){

			// Remove the match //
			CommandHandlersToNotify.erase(iter);
			return;
		}
	}
}

DLLEXPORT void Leviathan::CommandSender::_OnReleaseParentCommanders(){

	auto end = CommandHandlersToNotify.end();
	for(auto iter = CommandHandlersToNotify.begin(); iter != end; ++iter){

		(*iter)->RemoveMe(this);
	}

	// Clear all at once for hopefully better performance //
	CommandHandlersToNotify.clear();
}
// ------------------ CommandSender ------------------ //
DLLEXPORT void Leviathan::CommandSender::SendPrivateMessage(const std::string &message){
	if(!_OnSendPrivateMessage(message)){
		// Print to the log as a backup //
		Logger::Get()->Write("[MESSAGE] => "+GetNickname()+": "+message);
	}
}


