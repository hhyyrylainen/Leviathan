#ifndef LEVIATHAN_QUEUEDTASK
#define LEVIATHAN_QUEUEDTASK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <boost/function.hpp>
#include <boost/chrono/system_clocks.hpp>

//! Default value to pass for ignoring this setting //
#define TASK_MUSTBERAN_BEFORE_EXIT			0
//! Makes the task run before current frame ends
//! \todo Actually implement callbacks to ThreadingManager for Engine to call this and make this work
#define TASK_MUSTBERAN_BEFORE_FRAMEEND		1


namespace Leviathan{

	//! \brief Object passed to tasks which has common values
	//!
	//! This is passed to improve performance since querying the system clock multiple times by every task can be quite slow
	struct QueuedTaskCheckValues{
		//! \brief Constructs a new value holder and automatically fetches the CurrentTime value
		QueuedTaskCheckValues();


		//! Time the task set iteration started (may be slightly off, but it doesn't matter) //
		WantedClockType::time_point CurrentTime;
	};


	//! \brief Encapsulates a function that can later be ran in a free thread
	//! \warning Function passed to this class should be thread safe
	//! \warning This is not explicitly thread safe, it might be through Boost::Thread 
	class QueuedTask{
	public:
		//! Takes in the function which is ran when the Task is ran
		DLLEXPORT QueuedTask(boost::function<void ()> functorun);
		DLLEXPORT virtual ~QueuedTask();

		//! \brief Runs the stored function
		DLLEXPORT virtual void RunTask();

		// Functions for child classes to implement //

		//! \brief Function called by ThreadingManager before running this task
		//! \return By default returns always true, but child classes can perform various checks before returning
		DLLEXPORT virtual bool CanBeRan(const QueuedTaskCheckValues* const checkvalues);

		//! \brief Function called by ThreadingManager before certain events to make proper tasks finish before certain operations
		//! \return By default returns true when passed TASK_MUSTBERAN_BEFORE_EXIT, but child classes can store internal variables 
		//! to match only certain types
		DLLEXPORT virtual bool MustBeRanBefore(int eventtypeidentifier);


		//! \brief Function called by ThreadingManager AFTER running the task //
		//! \return By default returns always false (so will be removed from queue), but child classes can perform various checks or
		//! an internal state which holds information about repeating the task, such as repeat x times or repeat until success etc.
		//! \note This is guaranteed to be called only once per execution so this can be used to implement an execution times monitor
		DLLEXPORT virtual bool IsRepeating();

	private:
		//! \brief Provided for child classes to do something before running the function
		virtual void _PreFunctionRun();
		//! \brief Provides child classes a way to execute after running the function
		virtual void _PostFunctionRun();
		// ------------------------------------ //

		//! The function to run
		boost::function<void ()> FunctionToRun;

	};

	// ------------------ Specialized QueuedTasks for common operations ------------------ //

	//! \brief Encapsulates a function that can later be ran in a free thread
	//! \warning Function passed to this class should be thread safe
	class ConditionalTask : public QueuedTask{
	public:
		//! Constructs a task that can be controlled when it can be ran
		//! \param canberuncheck Is ran when CanBeRan is called, so it should be relatively cheap to call
		DLLEXPORT ConditionalTask(boost::function<void ()> functorun, boost::function<bool ()> canberuncheck);
		DLLEXPORT virtual ~ConditionalTask();


		//! \brief Calls the checking function to see if the task can be ran
		DLLEXPORT virtual bool CanBeRan(const QueuedTaskCheckValues* const checkvalues);

	protected:

		//! The function for checking if the task is allowed to be run
		boost::function<bool ()> TaskCheckingFunc;
	};

	//! \brief Encapsulates a function that is ran after a time period
	//! \warning Function passed to this class should be thread safe
	class DelayedTask : public QueuedTask{
	public:
		//! Constructs a task that can be controlled when it can be ran
		//! \param delaytime Is used to control when the task can run
		//! \see QueuedTask
		DLLEXPORT DelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &delaytime);
		//! Constructs a task that can be controlled when it can be ran
		//! \param executetime Is used to control when the task can run
		//! \see QueuedTask
		DLLEXPORT DelayedTask(boost::function<void ()> functorun, const WantedClockType::time_point &executetime);
		DLLEXPORT virtual ~DelayedTask();


		//! \brief Checks if the current time is past the time stamp
		//!
		//! Controlled by the value of ExecutionTime
		DLLEXPORT virtual bool CanBeRan(const QueuedTaskCheckValues* const checkvalues);

	protected:

		//! The time after which this task may be ran
		WantedClockType::time_point ExecutionTime;
	};


	//! \brief Encapsulates a function that is ran after a time period
	//! \warning Function passed to this class should be thread safe
	//! \warning The passed task will be repeatedly ran until SetRepeatStatus is called with false 
	//! (or ShouldRunAgain some other way set to false)
	//! \see SetRepeatStatus
	class RepeatingDelayedTask : public DelayedTask{
	public:
		//! Constructs a task that can be controlled when it can be ran and how many times it is ran
		//! \param bothdelays Is the time before first execution AND the time between following executions
		//! \see DelayedTask
		DLLEXPORT RepeatingDelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &bothdelays);
		//! Constructs a task that can be controlled when it can be ran and how many times it is ran
		//! \param initialdelay Is the time before first execution
		//! \param followingduration Is the time between following executions
		//! \see DelayedTask
		DLLEXPORT RepeatingDelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &initialdelay, const MicrosecondDuration &followingduration);

		DLLEXPORT virtual ~RepeatingDelayedTask();

		//! \brief Called by ThreadingManager to see if this task still wants to repeat
		//! \see SetRepeatStatus ShouldRunAgain
		DLLEXPORT virtual bool IsRepeating();

		//! \brief Sets the variable ShouldRunAgain to newvalue
		//! \note To call this from the task function use TaskThread::GetThreadSpecificThreadObject
		//! \see ThreadSpecificData ThreadSpecificData::QuickTaskAccess
		DLLEXPORT void SetRepeatStatus(bool newvalue);

	protected:
		//! \brief Used to update the time when to run the task again
		virtual void _PostFunctionRun();
		// ------------------------------------ //

		MicrosecondDuration TimeBetweenExecutions;

		//! Controls if the task should still run again
		bool ShouldRunAgain;
	};




	//! \brief Encapsulates a function that is ran after a time period and repeated certain amount
	//! \warning Function passed to this class should be thread safe
	class RepeatCountedDelayedTask : public DelayedTask{
	public:
		//! Constructs a task that can be controlled when it can be ran and how many times it is ran
		//! \param bothdelays Is the time before first execution AND the time between following executions
		//! \param repeatcount The task is ran this specified times, or the task calls StopRepeating before that
		//! \see DelayedTask
		DLLEXPORT RepeatCountedDelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &bothdelays, int repeatcount);
		//! Constructs a task that can be controlled when it can be ran and how many times it is ran
		//! \param initialdelay Is the time before first execution
		//! \param followingduration Is the time between following executions
		//! \param repeatcount The task is ran this specified times, or the task calls StopRepeating before that
		//! \see DelayedTask
		DLLEXPORT RepeatCountedDelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &initialdelay, const MicrosecondDuration 
			&followingduration, int repeatcount);

		DLLEXPORT virtual ~RepeatCountedDelayedTask();

		//! \brief Called by ThreadingManager to see if this task still wants to repeat
		//! \see SetRepeatStatus ShouldRunAgain
		DLLEXPORT virtual bool IsRepeating();

		//! \brief Stops repeating this function after this call
		//! \note To call this from the task function use TaskThread::GetThreadSpecificThreadObject
		//! \see ThreadSpecificData ThreadSpecificData::QuickTaskAccess
		DLLEXPORT void StopRepeating();

	protected:
		//! \brief Used to update the time when to run the task again
		virtual void _PostFunctionRun();
		// ------------------------------------ //

		MicrosecondDuration TimeBetweenExecutions;

		//! Holds the maximum run count
		int MaxRepeats;

		//! Keeps track of how many times the task has been ran
		//! \see IsRepeating
		int RepeatedCount;
	};

}
#endif