// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once
#include "Define.h"
// ------------------------------------ //
#include "QueuedTask.h"
#include "TaskThread.h"

#ifdef _WIN32
#include "WindowsInclude.h"
#endif //_WIN32

#include <list>
#include <vector>

#define DEFAULT_THREADS_PER_CORE 1

namespace Leviathan {

#ifdef LEVIATHAN_USING_OGRE
DLLEXPORT void RegisterOgreOnThread();
DLLEXPORT void UnregisterOgreOnThread();
#endif // LEVIATHAN_USING_OGRE

//! \todo Improve performance
void RunTaskQueuerThread(ThreadingManager* manager);

#ifdef _WIN32

void SetThreadName(TaskThread* thread, const std::string& name);
void SetThreadNameImpl(DWORD threadid, const std::string& name);
#else
void SetThreadName(TaskThread* thread, const std::string& name);

#endif // _WIN32


//! \brief Manages delayed execution of functions through use of QueuedTask and subclasses
class ThreadingManager : public ThreadSafe {
    friend void RunTaskQueuerThread(ThreadingManager* manager);

public:
    DLLEXPORT ThreadingManager(int basethreadspercore = DEFAULT_THREADS_PER_CORE);
    DLLEXPORT virtual ~ThreadingManager();

    //! Sets up the work queue
    DLLEXPORT virtual bool Init();
    //! \brief Checks has Init worked
    DLLEXPORT virtual bool CheckInit();

    //! This will take a long time, since it will wait until all tasks are done
    //! \todo Do something about unfinished tasks here
    DLLEXPORT virtual void Release();

    //! Adds a task to the queue
    DLLEXPORT void QueueTask(std::shared_ptr<QueuedTask> task);

    //! \brief Removes a task from the queue
    //! \pre The task is added with QueueTask
    //! \note If the task is currectly being executed current thread spinlocsk untill it is
    //! done
    DLLEXPORT bool RemoveFromQueue(std::shared_ptr<QueuedTask> task);

    //! \brief Removes specific tasks from the queue
    //! \pre The tasks are added with QueueTask and tasklist has the list of tasks to remove
    //! \post The tasklist is empty and the tasks won't be ran
    //! \note If the task is currectly being executed current thread spinlocsk untill it is
    //! done
    DLLEXPORT void RemoveTasksFromQueue(std::vector<std::shared_ptr<QueuedTask>>& tasklist);

    //! \brief Adds a task to the queue
    //! \param newdtask The task to queue, the pointer will be deleted by this
    DLLEXPORT FORCE_INLINE void QueueTask(QueuedTask* newdtask)
    {
        QueueTask(std::shared_ptr<QueuedTask>(newdtask));
    }

    //! This function waits for all tasks to complete
    DLLEXPORT void FlushActiveThreads();

    //! \brief Blocks until all queued tasks are finished
    //!
    //! \warning This function will ignore MustBeRanBefore return value by
    //! passing TASK_MUSTBERAN_BEFORE_EXIT
    //! \bug This doesn't properly handle tasks that are repeating
    DLLEXPORT void WaitForAllTasksToFinish();


    //! \brief Blocks until all threads are empty
    DLLEXPORT void WaitForWorkersToEmpty(Lock& guard);


    //! \brief Notifies the queuer thread to check task setting
    DLLEXPORT void NotifyQueuerThread();


    //! \brief Disallows repeating tasks to occur again
    //! \note This should only be called by the Engine class just before quitting
    DLLEXPORT void SetDisallowRepeatingTasks(bool disallow);

    //! \brief Sets the task queuer to discard all conditional tasks
    //! \note This should only be called by the Engine
    DLLEXPORT void SetDiscardConditionalTasks(bool discard);

    //! Called by work threads when they are done
    DLLEXPORT void NotifyTaskFinished(std::shared_ptr<QueuedTask> task);

    //! Makes the threads work with Ogre
    DLLEXPORT void MakeThreadsWorkWithOgre();

    //! Must be called if MakeThreadsWorkWithOgre has been called, BEFORE releasing graphics
    DLLEXPORT void UnregisterGraphics();

    DLLEXPORT static ThreadingManager* Get();

protected:
    bool AllowStartTasksFromQueue;
    bool StopProcessing;

    int WantedThreadCount;

    //! Used to allow QueuedTask::MustBeRanBefore function to work, shared between staticaccess
    //! worker thread and the main object
    int TaksMustBeRanBeforeState;

    //! Can tasks be repeated
    bool AllowRepeats;

    //! Controls whether tasks can be conditional. Setting this to false will remove all tasks
    //! that cannot be ran instantly
    bool AllowConditionalWait;


    //! List of the tasks queued by the application
    std::list<std::shared_ptr<QueuedTask>> WaitingTasks;
    std::condition_variable_any TaskQueueNotify;
    std::vector<std::shared_ptr<TaskThread>> UsableThreads;

    //! Thread used to set tasks to threads
    std::thread WorkQueueHandler;

    static ThreadingManager* staticaccess;
};
}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::ConditionalDelayedTask;
using Leviathan::ConditionalTask;
using Leviathan::DelayedTask;
using Leviathan::QueuedTask;
using Leviathan::RepeatCountedDelayedTask;
using Leviathan::RepeatingDelayedTask;
using Leviathan::ThreadingManager;
#endif
