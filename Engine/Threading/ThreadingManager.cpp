// ------------------------------------ //
#include "ThreadingManager.h"

#include "QueuedTask.h"
#include "Statistics/TimingMonitor.h"
#include "Utility/Convert.h"

#include <thread>
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

// ------------------ Utility functions for threads to run ------------------ //
// TODO: BSF may need some handling
namespace Leviathan {
void RegisterOgreOnThread() {}

DLLEXPORT void UnregisterOgreOnThread() {}
} // namespace Leviathan

// ------------------ ThreadingManager ------------------ //
DLLEXPORT Leviathan::ThreadingManager::ThreadingManager(int basethreadspercore
    /*= DEFAULT_THREADS_PER_CORE*/) :
    AllowStartTasksFromQueue(true),
    StopProcessing(false), TaksMustBeRanBeforeState(TASK_MUSTBERAN_BEFORE_EXIT),
    AllowRepeats(true), AllowConditionalWait(true)
{
    WantedThreadCount = std::thread::hardware_concurrency() * basethreadspercore;

    staticaccess = this;
}

DLLEXPORT Leviathan::ThreadingManager::~ThreadingManager()
{

    // Joins all threads before quitting //
    UsableThreads.clear();
    staticaccess = nullptr;
}

DLLEXPORT ThreadingManager* Leviathan::ThreadingManager::Get()
{
    return staticaccess;
}

ThreadingManager* Leviathan::ThreadingManager::staticaccess = NULL;
// ------------------------------------ //
DLLEXPORT bool Leviathan::ThreadingManager::Init()
{

    GUARD_LOCK();

    // Start the queuer //
    WorkQueueHandler = std::thread(RunTaskQueuerThread, this);


    // Start appropriate amount of threads //

    for(int i = 0; i < WantedThreadCount; i++) {


        UsableThreads.push_back(shared_ptr<TaskThread>(new TaskThread()));
    }

    return true;
}

DLLEXPORT bool Leviathan::ThreadingManager::CheckInit()
{
    // Check are there running threads //
    GUARD_LOCK();


    bool started = false;
    int loopcount = 0;

    std::this_thread::yield();

    // This might need to be repeated for a while //
    while(!started) {

        // Check that at least one thread is running //
        for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter) {
            // Check is this thread running //
            if((*iter)->HasStarted()) {
                // Something has started //
                started = true;

                // Set the thread names //
                int threadnumber = 0;
                for(auto iter2 = UsableThreads.begin(); iter2 != UsableThreads.end();
                    ++iter2) {

                    // Set the name //
                    SetThreadName(
                        (*iter2).get(), "Lev_Task_" + Convert::ToString(threadnumber));

                    threadnumber++;
                }

                return true;
            }
        }

        if(loopcount > 100) {
            // Sleep a bit
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if(++loopcount > 1000) {
#ifndef LEVIATHAN_UE_PLUGIN
            Logger::Get()->Error(
                "ThreadingManager: CheckInit: no threads have started, after 1000 loops");
#endif // LEVIATHAN_UE_PLUGIN

            // No threads running //
            return false;
        }

        std::this_thread::yield();
    }

    LEVIATHAN_ASSERT(0, "Shouldn't get out of that loop");
    return false;
}



DLLEXPORT void Leviathan::ThreadingManager::Release()
{
    // Disallow new tasks //
    {
        GUARD_LOCK();
        AllowStartTasksFromQueue = false;
    }

    // Wait for all to finish //
    // WaitForAllTasksToFinish();

    {
        GUARD_LOCK();
        StopProcessing = true;
    }
    // Wait for the queuer to exit //
    TaskQueueNotify.notify_all();
    WorkQueueHandler.join();

    // Tell all threads to quit //
    for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter) {
        (*iter)->NotifyKill();
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ThreadingManager::QueueTask(shared_ptr<QueuedTask> task)
{
    {
        GUARD_LOCK();

        WaitingTasks.push_back(task);
    }
    // Notify the thread //
    TaskQueueNotify.notify_all();
}

DLLEXPORT bool Leviathan::ThreadingManager::RemoveFromQueue(shared_ptr<QueuedTask> task)
{
    // Best case scenario is finding it in the wait queue //
    {
        GUARD_LOCK();

        auto end = WaitingTasks.end();
        for(auto iter = WaitingTasks.begin(); iter != end; ++iter) {

            if((*iter).get() == task.get()) {

                WaitingTasks.erase(iter);
                return true;
            }
        }
    }

    // The worse case is it having finished already //
    // And the worst case is it being currently executed //
    bool wasrunning = false;
    bool isrunning = false;

    do {

        isrunning = false;

        GUARD_LOCK();

        auto end = UsableThreads.end();
        for(auto iter = UsableThreads.begin(); iter != end; ++iter) {

            if((*iter)->IsRunningTask(task)) {

                isrunning = true;
                wasrunning = true;
                break;
            }
        }

    } while(isrunning);

    return wasrunning;
}

DLLEXPORT void Leviathan::ThreadingManager::RemoveTasksFromQueue(
    std::vector<shared_ptr<QueuedTask>>& tasklist)
{
    // Just go one by one and remove them all //
    for(size_t i = 0; i < tasklist.size(); i++) {

        RemoveFromQueue(tasklist[i]);
    }

    tasklist.clear();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ThreadingManager::FlushActiveThreads()
{
    // Disallow new tasks //
    GUARD_LOCK_NAME(lockit);
    AllowStartTasksFromQueue = false;


    bool allavailable = false;

    // We want to skip wait on loop //
    goto skipfirstwaitforthreadslabel;

    while(!allavailable) {

        // Wait for tasks to update //
        TaskQueueNotify.wait_for(lockit, std::chrono::milliseconds(10));

    skipfirstwaitforthreadslabel:


        // Set to true until a thread is busy //
        allavailable = true;

        for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter) {
            if((*iter)->HasRunningTask()) {
                allavailable = false;
                break;
            }
        }
    }

    // Now free //
}

DLLEXPORT void Leviathan::ThreadingManager::WaitForAllTasksToFinish()
{
    // Use this lock the entire function //
    GUARD_LOCK();

    // See if empty right now and loop until it is //
    while(true) {

        WaitForWorkersToEmpty(guard);

        // Unlock us for a second
        guard.unlock();
        // Make the queuer run //
        TaskQueueNotify.notify_all();
        guard.lock();

        WaitForWorkersToEmpty(guard);
        WaitForWorkersToEmpty(guard);

        if(WaitingTasks.empty())
            return;
    }

    WaitForWorkersToEmpty(guard);
}

DLLEXPORT void ThreadingManager::WaitForWorkersToEmpty(Lock& guard)
{

    // Wait for threads to empty up //
    bool allavailable = false;

    // We want to skip wait on loop //
    goto skipfirstwaitforthreadslabel2;

    while(!allavailable) {

        // Wait for tasks to update //
        TaskQueueNotify.wait_for(guard, std::chrono::milliseconds(1));

    skipfirstwaitforthreadslabel2:

        // Set to true until a thread is busy //
        allavailable = true;

        for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter) {
            if((*iter)->HasRunningTask()) {
                allavailable = false;
                break;
            }
        }
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ThreadingManager::NotifyTaskFinished(shared_ptr<QueuedTask> task)
{
    // We need locking for re-adding it //
    if(task->IsRepeating()) {
        // Add back to queue //
        GUARD_LOCK();

        // Or not if we should be quitting soon //
        if(AllowRepeats)
            WaitingTasks.push_back(task);
    }


    // We probably don't need to acquire a lock for this //
    TaskQueueNotify.notify_all();
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ThreadingManager::MakeThreadsWorkWithOgre()
{

    QUICKTIME_THISSCOPE;

    // Disallow new tasks //
    {
        GUARD_LOCK();
        AllowStartTasksFromQueue = false;
    }

    // Set our main thread's name //
    // SetThreadNameImpl(-1, "LeviathanMain");

    // Wait for tasks to finish //
    FlushActiveThreads();

    // All threads are now available //

    // TODO: if BSF thread setup is needed it should be here

    // Wait for threads to finish //
    FlushActiveThreads();

    // End registering functions //

    // Allow new tasks to run //
    {
        GUARD_LOCK();
        AllowStartTasksFromQueue = true;
    }
}

DLLEXPORT void Leviathan::ThreadingManager::UnregisterGraphics()
{
    // Wait for threads to finish //
    FlushActiveThreads();

    {
        GUARD_LOCK_NAME(lockit);

        for(auto iter = UsableThreads.begin(); iter != UsableThreads.end(); ++iter) {

            //(*iter)->SetTaskAndNotify(
            //    std::make_shared<QueuedTask>(std::bind(UnregisterOgreOnThread)));
            // Wait for it to end //
            // #ifdef __GNUC__
            // 	while((*iter)->HasRunningTask()){
            // 		try{
            // 			TaskQueueNotify.wait_for(lockit, std::chrono::milliseconds(50));
            // 		}
            // 		catch(...){
            // 			LOG_WARNING("ThreadingManager: MakeThreadsWorkWithOgre: "
            //                 "linux fix wait interrupted");
            // 		}
            // 	}
            // #endif
        }
    }

    FlushActiveThreads();

    // Allow new tasks to run //
    {
        GUARD_LOCK();
        AllowStartTasksFromQueue = true;
    }
}
// ------------------------------------ //
DLLEXPORT void Leviathan::ThreadingManager::NotifyQueuerThread()
{

    TaskQueueNotify.notify_all();
}

DLLEXPORT void Leviathan::ThreadingManager::SetDisallowRepeatingTasks(bool disallow)
{
    AllowRepeats = disallow;
}

DLLEXPORT void Leviathan::ThreadingManager::SetDiscardConditionalTasks(bool discard)
{
    AllowConditionalWait = !discard;
}
// ------------------------------------ //
void Leviathan::RunTaskQueuerThread(ThreadingManager* manager)
{

    // Lock the object //
    GUARD_LOCK_OTHER(manager);

    while(!manager->StopProcessing) {

        // Wait until task queue needs work //
        manager->TaskQueueNotify.wait_for(guard, std::chrono::milliseconds(100));

        // Quickly continue if it is empty //
        if(!manager->AllowStartTasksFromQueue || manager->WaitingTasks.empty()) {

            continue;
        }

        // Keep iterator consistent with the whole loop, (to avoid excessive calling of
        // CanBeRan) //
        auto taskiter = manager->WaitingTasks.begin();
        // Used to iterate again, but just checking if they can be ran (allows more important
        // tasks to run first) //
        auto nonimportantiter = manager->WaitingTasks.begin();

        // We need some common values for tasks to use for checking if they can run //
        QueuedTaskCheckValues commontaskcheck;

        // Find an empty thread and queue tasks //
        for(auto iter = manager->UsableThreads.begin(); iter != manager->UsableThreads.end();
            ++iter) {

            if(!(*iter)->HasRunningTask()) {

                // Break if no tasks //
                if(manager->WaitingTasks.empty())
                    break;

                // Queue a task //
                shared_ptr<QueuedTask> tmptask;

                // Try to find a suitable one //
                for(; taskiter != manager->WaitingTasks.end();) {

                    // Check does the task want to run now //
                    if((*taskiter)->MustBeRanBefore(manager->TaksMustBeRanBeforeState)) {
                        // Check is allowed to run //
                        if((*taskiter)->CanBeRan(&commontaskcheck)) {
                            // Run it! //
                            tmptask = (*taskiter);
                            // Erase, might be temporary //
                            taskiter = manager->WaitingTasks.erase(taskiter);

                            // Just to be safe, TODO: performance could be improved //
                            nonimportantiter = taskiter;

                            break;
                        } else if(!manager->AllowConditionalWait) {
                            // Discard it //
                            taskiter = manager->WaitingTasks.erase(taskiter);
                            // Just to be safe, TODO: performance could be improved //
                            nonimportantiter = taskiter;
                        }
                    } else if(!manager->AllowConditionalWait) {
                        // Discard it //
                        taskiter = manager->WaitingTasks.erase(taskiter);
                        // Just to be safe, TODO: performance could be improved //
                        nonimportantiter = taskiter;
                    }

                    ++taskiter;
                }

                if(!tmptask) {
                    // Check with the other iterator, too //
                    for(; nonimportantiter != manager->WaitingTasks.end();) {
                        // Check is allowed to run //
                        if((*nonimportantiter)->CanBeRan(&commontaskcheck)) {
                            // Run it! //
                            tmptask = (*nonimportantiter);
                            // Erase, might be temporary //
                            nonimportantiter = manager->WaitingTasks.erase(nonimportantiter);

                            // Just to be safe, TODO: performance could be improved //
                            taskiter = nonimportantiter;

                            break;
                        } else if(!manager->AllowConditionalWait) {
                            // Discard it //
                            nonimportantiter = manager->WaitingTasks.erase(taskiter);

                            // Just to be safe, TODO: performance could be improved //
                            taskiter = nonimportantiter;
                        }

                        ++nonimportantiter;
                    }
                }

                // If still nothing, nothing cannot run //
                if(!tmptask)
                    break;

                // This won't actually finish it so to re-queue it, if it repeats, we use the
                // callback called when it is finished
                (*iter)->SetTaskAndNotify(tmptask);
            }
        }
    }
}
// ------------------------------------ //
#ifdef _WIN32

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void Leviathan::SetThreadName(TaskThread* thread, const string& name)
{
    // Skip this if there is no debugger //
    if(!IsDebuggerPresent())
        return;

    // Get the native handle //
    DWORD nativehandle = GetThreadId(thread->GetInternalThreadObject().native_handle());

    SetThreadNameImpl(nativehandle, name);
}

void Leviathan::SetThreadNameImpl(DWORD threadid, const string& name)
{
    // Do this trick as shown on MSDN //
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    // Set the name //
    info.szName = name.c_str();
    info.dwThreadID = threadid;
    info.dwFlags = 0;

    __try {
        RaiseException(
            MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }
}
#elif __linux

void Leviathan::SetThreadName(TaskThread* thread, const string& name)
{

    pthread_setname_np(thread->GetInternalThreadObject().native_handle(), name.c_str());
}

#else
#error Do the Mac os thread name



#endif // _WIN32
