// ------------------------------------ //
#include "TaskThread.h"

#include "ThreadingManager.h"
#include "angelscript.h"
#include "OgreRoot.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

void Leviathan::RunNewThread(TaskThread* thisthread){
	// First create the thread specific ptr object //
	TaskThread::ThreadThreadPtr = make_shared<ThreadSpecificData>(thisthread);

	// We want to have a lock always when running this function //
	GUARD_LOCK(thisthread);

	// Register the thread //
	{
		GUARD_LOCK_OTHER_NAME(thisthread, guard2);
		thisthread->_NewThreadEntryRegister(guard2);
		thisthread->StartUpDone = true;
	}

	// Run and run tasks, while alive //
	while(!thisthread->KillSelf){
		// Skip wait if has a task //
		if(!thisthread->SetTask){

			// Wait until something happens //
			thisthread->ThreadNotify.wait(guard);
		}

		// Check stuff //
		if(thisthread->SetTask){
			// Copy our task to our data object //
			TaskThread::ThreadThreadPtr->QuickTaskAccess = thisthread->SetTask;

			// Unlock for running //
			guard.unlock();

			// Run the task //
			thisthread->SetTask->RunTask();

			// Re-lock for changing around //
			guard.lock();

			// Set our task away //
			thisthread->SetTask.reset();

			guard.unlock();
			// Notify run finished //
			ThreadingManager::Get()->NotifyTaskFinished(
                TaskThread::ThreadThreadPtr->QuickTaskAccess);
			// We might have already gotten a new task //
			guard.lock();
		}
	}

	{
		GUARD_LOCK_OTHER_NAME(thisthread, guard2);
		// Unregister the thread //
		thisthread->_ThreadEndClean(guard2);
	}
}

// ------------------ TaskThread ------------------ //
DLLEXPORT Leviathan::TaskThread::TaskThread() : StartUpDone(false), KillSelf(false){
	// Start the thread //
	ThisThread = std::thread(std::bind(RunNewThread, this));
}

DLLEXPORT Leviathan::TaskThread::~TaskThread(){
	// We want the thread to actually end sometime soon //
	NotifyKill();
	ThisThread.join();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::TaskThread::NotifyKill(Lock &guard){
	// Verify lock //
	VerifyLock(guard);

	// Set as quitting //
	KillSelf = true;

	// Notify the thread //
	NotifyThread();
}

DLLEXPORT void Leviathan::TaskThread::NotifyKill(){
	// Lock and call the other //
	GUARD_LOCK();
	NotifyKill(guard);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::TaskThread::NotifyThread(){
	ThreadNotify.notify_all();
}

void Leviathan::TaskThread::_NewThreadEntryRegister(Lock &guard){
	
}

void Leviathan::TaskThread::_ThreadEndClean(Lock &guard){
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
	GUARD_LOCK();

	// Set task //
	SetTask = task;

	// Notify //
	NotifyThread();
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::TaskThread::HasStarted(){
	// Get lock to wait for any possible action to finish //
	GUARD_LOCK();

	return StartUpDone;
}

DLLEXPORT bool Leviathan::TaskThread::HasRunningTask(){
	// Get lock to wait for any possible action to finish //
	GUARD_LOCK();

	return SetTask.get() != NULL;
}
// ------------------------------------ //
DLLEXPORT bool Leviathan::TaskThread::IsRunningTask(shared_ptr<QueuedTask> task) const{

    GUARD_LOCK();
    return SetTask.get() == task.get();
}
// ------------------------------------ //
DLLEXPORT boost::thread& Leviathan::TaskThread::GetBoostThreadObject(){
	return ThisThread;
}

DLLEXPORT ThreadSpecificData* Leviathan::TaskThread::GetThreadSpecificThreadObject(){
	return ThreadThreadPtr.get();
}

std::shared_ptr<ThreadSpecificData> TaskThread::ThreadThreadPtr;

// ------------------ ThreadSpecificData ------------------ //
ThreadSpecificData::ThreadSpecificData(TaskThread* threadptr) : ThreadObject(threadptr){

}
