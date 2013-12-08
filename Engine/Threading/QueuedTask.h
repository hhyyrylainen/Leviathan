#ifndef LEVIATHAN_QUEUEDTASK
#define LEVIATHAN_QUEUEDTASK
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include <boost\function.hpp>


namespace Leviathan{

	class QueuedTask{
	public:
		// Takes in the function which is ran when the Task is ran //
		DLLEXPORT QueuedTask(boost::function<void> functorun);
		DLLEXPORT virtual ~QueuedTask();


		DLLEXPORT void RunTask();

	private:

		virtual void _PreFunctionRun();
		// ------------------------------------ //

		// The function to run //
		boost::function<void> FunctionToRun;

	};

}
#endif