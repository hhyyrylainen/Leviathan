// ------------------------------------ //
#include "CommandHandler.h"

#include "Iterators/StringIterator.h"
#include "Threading/ThreadingManager.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

//! \brief Runs the thing
//! \param sender The sender to pass to the handler, this will be verified to be still valid before usage
void RunCustomHandler(shared_ptr<CustomCommandHandler> handler, std::shared_ptr<string> command,
    CommandSender* sender)
{

	Lock cmdlock;

	auto cmdhandler = CommandHandler::Get(cmdlock);

	// Cannot do anything if the handler no longer exist //
	if(!cmdhandler)
		return;


	// Check that the sender is still valid //
	Lock senderlock;
	if(!cmdhandler->IsSenderStillValid(cmdlock, sender, senderlock)){

		// it isn't there anymore //
		return;
	}

	handler->ExecuteCommand(*command, sender);

	// The sender is now no longer required //
	cmdhandler->SenderNoLongerRequired(cmdlock, sender, senderlock);
}



// ------------------ CommandHandler ------------------ //
DLLEXPORT Leviathan::CommandHandler::CommandHandler(NetworkServerInterface* owneraccess) : Owner(owneraccess){
	Staticaccess = this;
}

DLLEXPORT Leviathan::CommandHandler::~CommandHandler(){
	{
		Lock lock(StaticDeleteMutex);
		Staticaccess = NULL;
	}

	GUARD_LOCK();

	CustomHandlers.clear();

	// All the pointers have to be removed before deleting this //
	_LetGoOfAll(guard);
}

CommandHandler* Leviathan::CommandHandler::Staticaccess;
Mutex Leviathan::CommandHandler::StaticDeleteMutex;


DLLEXPORT CommandHandler* Leviathan::CommandHandler::Get(Lock &lockereceiver){
	Lock lock(StaticDeleteMutex);
	if(Staticaccess){
		
		lockereceiver = move(Locker::Unique(Staticaccess->ObjectsLock));
	}

	return Staticaccess;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::QueueCommand(const string &command,
    CommandSender* issuer, Lock &issuerlock)
{
	GUARD_LOCK();

	// Get the first word //
	StringIterator itr(new UTF8DataIterator(command));

	// Only skip a single / if there is one //
	if(itr.GetCharacter() == '/')
		itr.MoveToNext();

	// Get the first word //
	auto firstword = itr.GetNextCharacterSequence<string>(UNNORMALCHARACTER_TYPE_LOWCODES);

	if(!firstword || firstword->empty())
		return;



	// Find a command handler that provides handling for this command //

	// First check the default commands //
	if(IsThisDefaultCommand(*firstword)){

		// Queue a default command execution //
		DEBUG_BREAK;

		// Default will also need this //
		_AddSender(issuer, guard, issuerlock);
		return;
	}

	// Loop through and check the custom command handlers //
	auto end = CustomHandlers.end();
	for(auto iter = CustomHandlers.begin(); iter != end; ++iter){

		if((*iter)->CanHandleCommand(*firstword)){
			// Queue the command handler //
			ThreadingManager::Get()->QueueTask(new QueuedTask(std::bind(&RunCustomHandler, *iter, 
				make_shared<string>(command), issuer)));


			// And take good care of the object while the command handler is waiting //
			_AddSender(issuer, guard, issuerlock);
			return;
		}
	}

	// Couldn't find a handler for command //
	issuer->SendPrivateMessage("Unknown command \""+*firstword+"\"");
}

DLLEXPORT void Leviathan::CommandHandler::RemoveMe(CommandSender* object){
	GUARD_LOCK();

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

DLLEXPORT bool Leviathan::CommandHandler::IsSenderStillValid(Lock &guard, CommandSender* checkthis,
    Lock &retlock)
{
	// Check is it still in the list //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){
		if(*iter == checkthis){
			// It is still there //
			retlock = move(Locker::Unique((*iter)->ObjectsLock));
			return true;
		}
	}

	// Not found //
	return false;
}

// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::UpdateStatus(){
	GUARD_LOCK();


}
// ------------------------------------ //
void Leviathan::CommandHandler::_LetGoOfAll(Lock &guard){
	VerifyLock(guard);

	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){

        GUARD_LOCK_OTHER_NAME((*iter), lock);
		(*iter)->EndOwnership(lock, this);
	}

	// Clear them all at once //
	SendersInUse.clear();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::SenderNoLongerRequired(Lock &guard,
    CommandSender* checkthis, Lock &stillgotthis)
{

	// Remove from the vector //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){

		if(*iter == checkthis){
			// Notify it //
			(*iter)->EndOwnership(stillgotthis, this);

			// Remove the match //
			SendersInUse.erase(iter);
			return;
		}
	}
}

void Leviathan::CommandHandler::_AddSender(CommandSender* object, Lock &guard, Lock &objectlock){
	VerifyLock(guard);

	// Notify the object //
	object->StartOwnership(objectlock, this);

	// Add to the list //
	SendersInUse.push_back(object);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::CommandHandler::RegisterCustomCommandHandler(
    shared_ptr<CustomCommandHandler> handler)
{
	// Might be unnecessary to check this, but it's a way to return false sometimes //
	if(!handler)
		return false;

	CustomHandlers.push_back(handler);
	return true;
}

DLLEXPORT bool Leviathan::CommandHandler::IsThisDefaultCommand(const string &firstword) const{

	// Didn't match any of the strings, cannot be //
	return false;
}
// ------------------ CommandSender ------------------ //
DLLEXPORT void Leviathan::CommandSender::StartOwnership(Lock &guard, CommandHandler* commander){

	// Just add to the list //
	CommandHandlersToNotify.push_back(commander);
}

DLLEXPORT void Leviathan::CommandSender::EndOwnership(Lock &guard, CommandHandler* which){

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

DLLEXPORT void Leviathan::CommandSender::_OnReleaseParentCommanders(Lock &guard){
	VerifyLock(guard);

	auto end = CommandHandlersToNotify.end();
	for(auto iter = CommandHandlersToNotify.begin(); iter != end; ++iter){

		(*iter)->RemoveMe(this);
	}

	// Clear all at once for hopefully better performance //
	CommandHandlersToNotify.clear();
}
// ------------------ CommandSender ------------------ //
DLLEXPORT void Leviathan::CommandSender::SendPrivateMessage(const string &message){
	if(!_OnSendPrivateMessage(message)){
		// Print to the log as a backup //
		Logger::Get()->Write("[MESSAGE] => "+GetNickname()+": "+message);
	}
}


