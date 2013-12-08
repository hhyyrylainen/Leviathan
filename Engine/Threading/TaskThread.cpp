#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TASKTHREAD
#include "TaskThread.h"
#endif
#include "ThreadingManager.h"
using namespace Leviathan;
// ------------------------------------ //

void RunNewThread(TaskThread* thisthread){
	// We want to have a lock always when running this function //
	ObjectLock guard(*thisthread);

	// Register the thread //
	thisthread->_NewThreadEntryRegister(guard);
	thisthread->StartUpDone = true;

	// Run and run tasks, while alive //
	if(!thisthread->KillSelf){
		// Wait until something happens //
		thisthread->ThreadNotify.wait(guard);

		// Check stuff //
		if(thisthread->SetTask){
			// Run the task //
			thisthread->SetTask->RunTask();
			// Notify run finished //
			ThreadingManager::Get()->NotifyTaskFinished(thisthread->SetTask);
			thisthread->SetTask.reset();
		}
	}


	// Unregister the thread //
	thisthread->_ThreadEndClean(guard);
}

// ------------------ TaskThread ------------------ //
DLLEXPORT Leviathan::TaskThread::TaskThread() : StartUpDone(false), KillSelf(false){
	// Start the thread //
	ThisThread = boost::thread(boost::bind(RunNewThread, this));
}

DLLEXPORT Leviathan::TaskThread::~TaskThread(){
	// We want the thread to actually end sometime soon //
	NotifyKill();
	ThisThread.join();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::TaskThread::NotifyKill(ObjectLock &guard){
	// Verify lock //
	VerifyLock(guard);

	// Set as quitting //
	KillSelf = true;

	// Notify the thread //
	NotifyThread();
}

DLLEXPORT void Leviathan::TaskThread::NotifyKill(){
	// Lock and call the other //
	ObjectLock guard(*this);
	NotifyKill(guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::TaskThread::NotifyThread(){
	ThreadNotify.notify_all();
}

void Leviathan::TaskThread::_NewThreadEntryRegister(ObjectLock &guard){

}

void Leviathan::TaskThread::_ThreadEndClean(ObjectLock &guard){
	// Release script resources //
	if(asThreadCleanup() < 0){

		Logger::Get()->Error(L"Releasing threads while scripts are running!");
	}
	// Release Ogre //
	Ogre::Root::getSingleton().getRenderSystem()->unregisterThread();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::TaskThread::SetTaskAndNotify(shared_ptr<QueuedTask> task){
	ObjectLock guard(*this);

	// Set task //
	SetTask = task;

	// Notify //
	NotifyThread();
}
// ------------------------------------ //


