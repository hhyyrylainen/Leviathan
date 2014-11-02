#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TASKTHREAD
#include "TaskThread.h"
#endif
#include "ThreadingManager.h"
#include "angelscript.h"
#include "OgreRoot.h"
using namespace Leviathan;
// ------------------------------------ //

void Leviathan::RunNewThread(TaskThread* thisthread){
	// First create the thread specific ptr object //
	TaskThread::ThreadThreadPtr.reset(new ThreadSpecificData(thisthread));

	// We want to have a lock always when running this function //
	UNIQUE_LOCK_OBJECT(thisthread);

	// Register the thread //
	{
		GUARD_LOCK_OTHER_OBJECT(thisthread);
		thisthread->_NewThreadEntryRegister(guard);
		thisthread->StartUpDone = true;
	}

	// Run and run tasks, while alive //
	while(!thisthread->KillSelf){
		// Skip wait if has a task //
		if(!thisthread->SetTask){

			// Wait until something happens //
			thisthread->ThreadNotify.wait(lockit);
		}

		// Check stuff //
		if(thisthread->SetTask){
			// Copy our task to our data object //
			TaskThread::ThreadThreadPtr->QuickTaskAccess = thisthread->SetTask;

			// Unlock for running //
			lockit.unlock();

			// Run the task //
			thisthread->SetTask->RunTask();

			// Re-lock for changing around //
			lockit.lock();

			// Set our task away //
			thisthread->SetTask.reset();

			lockit.unlock();
			// Notify run finished //
			ThreadingManager::Get()->NotifyTaskFinished(TaskThread::ThreadThreadPtr->QuickTaskAccess);
			// We might have already gotten a new task //
			lockit.lock();
		}
	}

	{
		GUARD_LOCK_OTHER_OBJECT(thisthread);
		// Unregister the thread //
		thisthread->_ThreadEndClean(guard);
	}
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
	GUARD_LOCK_THIS_OBJECT();
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
	// Release Ogre (if Ogre is still active) //
	Ogre::Root* tmproot = Ogre::Root::getSingletonPtr();

	if(tmproot)
		tmproot->getRenderSystem()->unregisterThread();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::TaskThread::SetTaskAndNotify(shared_ptr<QueuedTask> task){
	GUARD_LOCK_THIS_OBJECT();

	// Set task //
	SetTask = task;

	// Notify //
	NotifyThread();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::TaskThread::HasStarted(){
	// Get lock to wait for any possible action to finish //
	GUARD_LOCK_THIS_OBJECT();

	return StartUpDone;
}

DLLEXPORT bool Leviathan::TaskThread::HasRunningTask(){
	// Get lock to wait for any possible action to finish //
	GUARD_LOCK_THIS_OBJECT();

	return SetTask.get() != NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::TaskThread::IsRunningTask(shared_ptr<QueuedTask> task) const{

    GUARD_LOCK_THIS_OBJECT();
    return SetTask.get() == task.get();
}
// ------------------------------------ //
DLLEXPORT boost::thread& Leviathan::TaskThread::GetBoostThreadObject(){
	return ThisThread;
}

DLLEXPORT ThreadSpecificData* Leviathan::TaskThread::GetThreadSpecificThreadObject(){
	return ThreadThreadPtr.get();
}

boost::thread_specific_ptr<ThreadSpecificData> Leviathan::TaskThread::ThreadThreadPtr;

Leviathan::ThreadSpecificData::ThreadSpecificData(TaskThread* threadptr) : ThreadObject(threadptr){

}
