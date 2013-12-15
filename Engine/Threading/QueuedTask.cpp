#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_QUEUEDTASK
#include "QueuedTask.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::QueuedTask::QueuedTask(boost::function<void ()> functorun) : FunctionToRun(functorun){

}

DLLEXPORT Leviathan::QueuedTask::~QueuedTask(){

}
// ------------------------------------ //
DLLEXPORT void Leviathan::QueuedTask::RunTask(){
	// Run the function //
	_PreFunctionRun();

	FunctionToRun();
}
// ------------------------------------ //
void Leviathan::QueuedTask::_PreFunctionRun(){

}
// ------------------------------------ //

// ------------------------------------ //




