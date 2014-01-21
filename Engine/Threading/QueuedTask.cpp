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
DLLEXPORT bool Leviathan::QueuedTask::CanBeRan(){
	return true;
}

DLLEXPORT bool Leviathan::QueuedTask::MustBeRanBefore(int eventtypeidentifier){
	return eventtypeidentifier == TASK_MUSTBERAN_BEFORE_EXIT;
}
// ------------------------------------ //



// ------------------ ConditionalTask ------------------ //
DLLEXPORT Leviathan::ConditionalTask::ConditionalTask(boost::function<void ()> functorun, boost::function<bool ()> canberuncheck) : 
	QueuedTask(functorun), TaskCheckingFunc(canberuncheck)
{

}

DLLEXPORT Leviathan::ConditionalTask::~ConditionalTask(){

}

DLLEXPORT bool Leviathan::ConditionalTask::CanBeRan(){
	return TaskCheckingFunc();
}
