#ifndef LEVIATHAN_TASKTHREAD
#define LEVIATHAN_TASKTHREAD
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <boost\thread\detail\thread.hpp>
#include "Common\ThreadSafe.h"
#include "QueuedTask.h"

namespace Leviathan{

	void RunNewThread(TaskThread* thisthread);

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
	};

}
#endif