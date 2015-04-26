#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_COMMANDHANDLER
#include "CommandHandler.h"
#endif
#include "Iterators/StringIterator.h"
#include "Threading/ThreadingManager.h"
using namespace Leviathan;
// ------------------------------------ //

//! \brief Runs the thing
//! \param sender The sender to pass to the handler, this will be verified to be still valid before usage
void RunCustomHandler(shared_ptr<CustomCommandHandler> handler, std::shared_ptr<string> command, CommandSender* sender){

	unique_ptr<Lock> cmdlock;

	auto cmdhandler = CommandHandler::Get(cmdlock);

	// Cannot do anything if the handler no longer exist //
	if(!cmdhandler)
		return;


	// Check that the sender is still valid //
	unique_ptr<Lock> senderlock;
	if(!cmdhandler->IsSenderStillValid(sender, senderlock)){

		// it isn't there anymore //
		return;
	}

	handler->ExecuteCommand(*command, sender);

	// The sender is now no longer required //
	cmdhandler->SenderNoLongerRequired(sender, senderlock);
}



// ------------------ CommandHandler ------------------ //
DLLEXPORT Leviathan::CommandHandler::CommandHandler(NetworkServerInterface* owneraccess) : Owner(owneraccess){
	Staticaccess = this;
}

DLLEXPORT Leviathan::CommandHandler::~CommandHandler(){
	{
		boost::unique_lock<boost::mutex> lock(StaticDeleteMutex);
		Staticaccess = NULL;
	}

	GUARD_LOCK();

	CustomHandlers.clear();

	// All the pointers have to be removed before deleting this //
	_LetGoOfAll(guard);
}

CommandHandler* Leviathan::CommandHandler::Staticaccess;
boost::mutex Leviathan::CommandHandler::StaticDeleteMutex;


DLLEXPORT CommandHandler* Leviathan::CommandHandler::Get(unique_ptr<Lock> &lockereceiver){
	boost::unique_lock<boost::mutex> lock(StaticDeleteMutex);
	if(Staticaccess){
		
		GUARD_LOCK_OTHER_UNIQUE_PTR_NAME(Staticaccess, olock);
		lockereceiver.swap(olock);
	}

	return Staticaccess;
}
// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::QueueCommand(const string &command, CommandSender* issuer){
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
		_AddSender(issuer, guard);
		return;
	}

	// Loop through and check the custom command handlers //
	auto end = CustomHandlers.end();
	for(auto iter = CustomHandlers.begin(); iter != end; ++iter){

		if((*iter)->CanHandleCommand(*firstword)){
			// Queue the command handler //
			ThreadingManager::Get()->QueueTask(new QueuedTask(boost::bind(&RunCustomHandler, *iter, 
				make_shared<string>(command), issuer)));


			// And take good care of the object while the command handler is waiting //
			_AddSender(issuer, guard);
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

DLLEXPORT bool Leviathan::CommandHandler::IsSenderStillValid(CommandSender* checkthis, std::unique_ptr<Lock> &retlock){
	GUARD_LOCK();

	// Check is it still in the list //
	auto end = SendersInUse.end();
	for(auto iter = SendersInUse.begin(); iter != end; ++iter){
		if(*iter == checkthis){
			// It is still there //
			GUARD_LOCK_OTHER_UNIQUE_PTR_NAME((*iter), tmplock);
			retlock.swap(tmplock);
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

		(*iter)->EndOwnership(this);
	}

	// Clear them all at once //
	SendersInUse.clear();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::CommandHandler::SenderNoLongerRequired(CommandSender* checkthis, const std::unique_ptr<Lock> &stillgotthis){
	GUARD_LOCK();

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

void Leviathan::CommandHandler::_AddSender(CommandSender* object, Lock &guard){
	VerifyLock(guard);

	// Notify the object //
	object->StartOwnership(this);

	// Add to the list //
	SendersInUse.push_back(object);
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::CommandHandler::RegisterCustomCommandHandler(shared_ptr<CustomCommandHandler> handler){
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
DLLEXPORT void Leviathan::CommandSender::StartOwnership(CommandHandler* commander){
	GUARD_LOCK();

	// Just add to the list //
	CommandHandlersToNotify.push_back(commander);
}

DLLEXPORT void Leviathan::CommandSender::EndOwnership(CommandHandler* which){
	GUARD_LOCK();

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
		Logger::Get()->Write(L"[MESSAGE] => "+Convert::Utf8ToUtf16(GetNickname())+L": "+Convert::Utf8ToUtf16(message));
	}
}


