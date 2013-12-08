#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_THREADINGMANAGER
#include "ThreadingManager.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::ThreadingManager::ThreadingManager(int basethreadspercore /*= DEFAULT_THREADS_PER_CORE*/){

}

DLLEXPORT Leviathan::ThreadingManager::~ThreadingManager(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ThreadingManager::Init(){

}

DLLEXPORT void Leviathan::ThreadingManager::Release(){

}
// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT void Leviathan::ThreadingManager::QueueTask(shared_ptr<QueuedTask> task){

}

DLLEXPORT void Leviathan::ThreadingManager::FlushActiveThreads(){

}

DLLEXPORT void Leviathan::ThreadingManager::WaitForAllTasksToFinish(){

}

DLLEXPORT void Leviathan::ThreadingManager::NotifyTaskFinished(shared_ptr<QueuedTask> task){

}
// ------------------------------------ //

DLLEXPORT void Leviathan::ThreadingManager::MakeThreadsWorkWithOgre(){
	// Disallow new threads //
	{
		ObjectLock guard(*this);
		AllowStartTasksFromQueue = false;
	}

	// Wait for tasks to finish //
	FlushActiveThreads();

	// All threads are now available //

	// Call pre register function //
	Ogre::Root::getSingleton().getRenderSystem()->preExtraThreadsStarted();

	// Set the threads to run the register methods //
	{
		ObjectLock guard(*this);

		for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter){
			(*iter)->SetTaskAndNotify(shared_ptr<QueuedTask>(new QueuedTask(boost::bind(&Ogre::RenderSystem::registerThread, Ogre::Root::getSingleton().getRenderSystem()))));
		}
	}
	
	// Wait for threads to finish //
	FlushActiveThreads();

	// End registering functions //
	Ogre::Root::getSingleton().getRenderSystem()->postExtraThreadsStarted();

	// Allow new threads //
	{
		ObjectLock guard(*this);
		AllowStartTasksFromQueue = true;
	}
}

DLLEXPORT ThreadingManager* Leviathan::ThreadingManager::Get(){
	return staticaccess;
}







ThreadingManager* Leviathan::ThreadingManager::staticaccess = NULL;
