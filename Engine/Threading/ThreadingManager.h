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


	DLLEXPORT void RegisterOgreOnThread();

	//! \todo Improve performance
	void RunTaskQueuerThread(ThreadingManager* manager);

#ifdef _WIN32

	void SetThreadName(TaskThread* thread, const string &name);
	void SetThreadNameImpl(DWORD threadid, const string &name);
#endif // _WIN32


	//! \brief Manages delayed execution of functions through use of QueuedTask and subclasses
	class ThreadingManager : public EngineComponent, public ThreadSafe{
		friend void RunTaskQueuerThread(ThreadingManager* manager);
	public:
		DLLEXPORT ThreadingManager(int basethreadspercore = DEFAULT_THREADS_PER_CORE);
		DLLEXPORT ~ThreadingManager();

		//! Sets up the work queue
		DLLEXPORT virtual bool Init();
		//! \brief Checks has Init worked
		DLLEXPORT virtual bool CheckInit();

		//! This will take a long time, since it will wait until all tasks are done
		//! \todo Do something about unfinished tasks here
		DLLEXPORT virtual void Release();

		//! Adds a task to the queue
		DLLEXPORT void QueueTask(shared_ptr<QueuedTask> task);

		//! \brief Adds a task to the queue
		//! \param newdtask The task to queue, the pointer will be deleted by this
		DLLEXPORT FORCE_INLINE void QueueTask(QueuedTask* newdtask){
			QueueTask(shared_ptr<QueuedTask>(newdtask));
		}

		//! This function waits for all tasks to complete
		DLLEXPORT void FlushActiveThreads();

		//! \brief Blocks until all queued tasks are finished
		//!
		//! \warning This function will ignore MustBeRanBefore return value by passing TASK_MUSTBERAN_BEFORE_EXIT
		//! \bug This doesn't properly handle tasks that are repeating
		//! \todo Maybe add a global thread that runs the queuer once in a while
		DLLEXPORT void WaitForAllTasksToFinish();


		//! \brief Notifies the queuer thread to check task setting
		DLLEXPORT void NotifyQueuerThread();


		//! \brief Disallows repeating tasks to occur again
		//! \note This should only be called by the Engine class just before quitting
		DLLEXPORT void SetDisallowRepeatingTasks(bool disallow);

		//! \brief Sets the task queuer to discard all conditional tasks
		//! \note This should only be called by the Engine
		DLLEXPORT void SetDiscardConditionalTasks(bool discard);

		//! Called by work threads when they are done
		DLLEXPORT void NotifyTaskFinished(shared_ptr<QueuedTask> task);

		//! Makes the threads work with Ogre
		DLLEXPORT void MakeThreadsWorkWithOgre();


		DLLEXPORT static ThreadingManager* Get();
	protected:

		bool AllowStartTasksFromQueue;
		bool StopProcessing;

		int WantedThreadCount;

		//! Used to allow QueuedTask::MustBeRanBefore function to work, shared between staticaccess worker thread and the main object
		int TaksMustBeRanBeforeState;

		//! Can tasks be repeated
		bool AllowRepeats;

		//! Controls whether tasks can be conditional. Setting this to false will remove all tasks that cannot be ran instantly
		bool AllowConditionalWait;


		//! List of the tasks queued by the application
		std::list<shared_ptr<QueuedTask>> WaitingTasks;
		boost::condition_variable_any TaskQueueNotify;
		std::list<shared_ptr<TaskThread>> UsableThreads;

		//! Thread used to set tasks to threads
		boost::thread WorkQueueHandler;

		static ThreadingManager* staticaccess;
	};

}
#endif