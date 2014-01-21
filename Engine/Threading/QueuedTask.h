#ifndef LEVIATHAN_QUEUEDTASK
#define LEVIATHAN_QUEUEDTASK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <boost/function.hpp>

//! Default value to pass for ignoring this setting //
#define TASK_MUSTBERAN_BEFORE_EXIT			0
//! Makes the task run before current frame ends
//! \todo Actually implement callbacks to ThreadingManager for Engine to call this and make this work
#define TASK_MUSTBERAN_BEFORE_FRAMEEND		1


namespace Leviathan{

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
		DLLEXPORT virtual bool CanBeRan();

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


		//! \brief Calls the 
		DLLEXPORT virtual bool CanBeRan();

	protected:

		//! The function for checking if the task is allowed to be run
		boost::function<bool ()> TaskCheckingFunc;
	};


}
#endif