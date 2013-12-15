#ifndef LEVIATHAN_THREADINGMANAGER
#define LEVIATHAN_THREADINGMANAGER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "QueuedTask.h"
#include "TaskThread.h"


#define DEFAULT_THREADS_PER_CORE		2

namespace Leviathan{


	void RunTaskQueuerThread(ThreadingManager* manager);


	class ThreadingManager : public EngineComponent, public ThreadSafe{
		friend void RunTaskQueuerThread(ThreadingManager* manager);
	public:
		DLLEXPORT ThreadingManager(int basethreadspercore = DEFAULT_THREADS_PER_CORE);
		DLLEXPORT ~ThreadingManager();

		// Sets up the work queue //
		DLLEXPORT virtual bool Init();
		// This will take a long time, since it will wait until all tasks are done //
		DLLEXPORT virtual void Release();

		// Adds a task to the queue //
		DLLEXPORT void QueueTask(shared_ptr<QueuedTask> task);

		// This function waits for all tasks to complete //
		DLLEXPORT void FlushActiveThreads();

		DLLEXPORT void WaitForAllTasksToFinish();

		// Called by work threads when they are done //
		DLLEXPORT void NotifyTaskFinished(shared_ptr<QueuedTask> task);

		// Makes the threads work with Ogre //
		DLLEXPORT void MakeThreadsWorkWithOgre();


		DLLEXPORT static ThreadingManager* Get();
	protected:

		bool AllowStartTasksFromQueue;
		bool StopProcessing;

		int WantedThreadCount;

		std::list<shared_ptr<QueuedTask>> WaitingTasks;
		boost::condition_variable_any TaskQueueNotify;
		std::list<shared_ptr<TaskThread>> UsableThreads;

		// Thread used to set tasks to threads //
		boost::thread WorkQueueHandler;

		static ThreadingManager* staticaccess;
	};

}
#endif