// ------------------------------------ //
#include "TaskThread.h"

#include "ThreadingManager.h"
#ifdef LEVIATHAN_USING_ANGELSCRIPT
#include "angelscript.h"
#endif // LEVIATHAN_USING_ANGELSCRIPT

#ifdef ALLOW_INTERNAL_EXCEPTIONS
#include "Exceptions.h"
#endif //ALLOW_INTERNAL_EXCEPTIONS

using namespace Leviathan;
using namespace std;
// ------------------------------------ //

void Leviathan::RunNewThread(TaskThread* thisthread){
	// First create the thread specific ptr object //
	TaskThread::ThreadThreadPtr = make_shared<ThreadSpecificData>(thisthread);

	// We want to have a lock always when running this function //
	GUARD_LOCK_OTHER(thisthread);

	// Register the thread //
    thisthread->_NewThreadEntryRegister(guard);
    thisthread->StartUpDone = true;

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

            try{
                // Run the task //
                TaskThread::ThreadThreadPtr->QuickTaskAccess->RunTask();

            } catch(const Exception &e){

            #ifndef LEVIATHAN_UE_PLUGIN
                Logger::Get()->Error("TaskThread: task threw a Leviathan exception: ");
                e.PrintToLog();
            #else
                NOT_UNUSED(e);
            #endif //LEVIATHAN_UE_PLUGIN
                DEBUG_BREAK;

            } catch(const std::exception &e){

            #ifndef LEVIATHAN_UE_PLUGIN
                Logger::Get()->Error("TaskThread: task threw a generic exception: ");
                Logger::Get()->Write(string("\t> ")+e.what());
            #else
                NOT_UNUSED(e);
            #endif //LEVIATHAN_UE_PLUGIN

                DEBUG_BREAK;
            }

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

    // Unregister the thread //
    thisthread->_ThreadEndClean(guard);
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
#ifdef LEVIATHAN_USING_ANGELSCRIPT
	// Release script resources //
	if(asThreadCleanup() < 0){

		Logger::Get()->Error("Releasing threads while scripts are running!");
	}
#endif // LEVIATHAN_USING_ANGELSCRIPT
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
DLLEXPORT std::thread& Leviathan::TaskThread::GetInternalThreadObject(){
	return ThisThread;
}

DLLEXPORT ThreadSpecificData* Leviathan::TaskThread::GetThreadSpecificThreadObject(){
	return ThreadThreadPtr.get();
}

thread_local std::shared_ptr<ThreadSpecificData> TaskThread::ThreadThreadPtr;

// ------------------ ThreadSpecificData ------------------ //
ThreadSpecificData::ThreadSpecificData(TaskThread* threadptr) : ThreadObject(threadptr){

}
