#ifndef LEVIATHAN_TASKTHREAD
#define LEVIATHAN_TASKTHREAD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <boost/thread/detail/thread.hpp>
#include "Common/ThreadSafe.h"
#include "QueuedTask.h"
#include <boost/thread/tss.hpp>

namespace Leviathan{

	void RunNewThread(TaskThread* thisthread);

	struct ThreadSpecificData{
		ThreadSpecificData(TaskThread* threadptr);


		TaskThread* ThreadObject;
		shared_ptr<QueuedTask> QuickTaskAccess;
	};

	//! \brief Object used by ThreadingManager to easily create properly initialized threads
	class TaskThread : public ThreadSafe{
		friend void RunNewThread(TaskThread* thisthread);
	public:
		// Warning this may only be called by the main thread and while no tasks are running, since this will register the thread in various places //
		DLLEXPORT TaskThread();

		DLLEXPORT ~TaskThread();

		DLLEXPORT void SetTaskAndNotify(shared_ptr<QueuedTask> task);

		DLLEXPORT void NotifyKill(ObjectLock &guard);
		DLLEXPORT void NotifyKill();

		DLLEXPORT void NotifyThread();

		// Returns true if the thread has performed initialization //
		DLLEXPORT bool HasStarted();
		// Returns true if the thread has a task to run //
		DLLEXPORT bool HasRunningTask();

		//! \brief Returns thread specific data about QueuedTask and TaskThread object
		DLLEXPORT static ThreadSpecificData* GetThreadSpecificThreadObject();

	private:

		void _NewThreadEntryRegister(ObjectLock &guard);
		void _ThreadEndClean(ObjectLock &guard);

		// ------------------------------------ //

		// For notifying the thread //
		boost::condition_variable_any ThreadNotify;

		// The task needed to be completed //
		shared_ptr<QueuedTask> SetTask;

		bool StartUpDone;
		bool KillSelf;
		boost::thread ThisThread;

		// Stores the thread object for the thread to access //
		static boost::thread_specific_ptr<ThreadSpecificData> ThreadThreadPtr;
	};

}
#endif
