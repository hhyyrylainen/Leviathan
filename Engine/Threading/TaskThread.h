#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include <thread>
#include "Common/ThreadSafe.h"
#include "QueuedTask.h"
#include <condition_variable>

namespace Leviathan{

	void RunNewThread(TaskThread* thisthread);

	struct ThreadSpecificData{
		ThreadSpecificData(TaskThread* threadptr);

		TaskThread* ThreadObject;
        std::shared_ptr<QueuedTask> QuickTaskAccess;
	};

	//! \brief Object used by ThreadingManager to easily create properly initialized threads
	class TaskThread : public ThreadSafe{
		friend void RunNewThread(TaskThread* thisthread);
	public:
		//! \warning this may only be called by the main thread and while no tasks are running,
        //! since this will register the thread in various places
		DLLEXPORT TaskThread();

		DLLEXPORT ~TaskThread();

		DLLEXPORT void SetTaskAndNotify(std::shared_ptr<QueuedTask> task);

		DLLEXPORT void NotifyKill(Lock &guard);
		DLLEXPORT void NotifyKill();

		DLLEXPORT void NotifyThread();

		//! \brief Returns true if the thread has performed initialization
		DLLEXPORT bool HasStarted();
        
		//! \brief Returns true if the thread has a task to run
		DLLEXPORT bool HasRunningTask();

        //! \brief Returns true if the current task pointer matches the argument
        DLLEXPORT bool IsRunningTask(std::shared_ptr<QueuedTask> task) const;

		//! \brief Returns the internal ThisThread variable
		DLLEXPORT std::thread& GetInternalThreadObject();

		//! \brief Returns thread specific data about QueuedTask and TaskThread object
		DLLEXPORT static ThreadSpecificData* GetThreadSpecificThreadObject();



	private:

		void _NewThreadEntryRegister(Lock &guard);
		void _ThreadEndClean(Lock &guard);

		// ------------------------------------ //

		// For notifying the thread //
		std::condition_variable ThreadNotify;

		// The task needed to be completed //
        std::shared_ptr<QueuedTask> SetTask;

		bool StartUpDone;
		bool KillSelf;
		std::thread ThisThread;

		// Stores the thread object for the thread to access //
		static thread_local std::shared_ptr<ThreadSpecificData> ThreadThreadPtr;
	};

}

