#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_QUEUEDTASK
#include "QueuedTask.h"
#endif
#include "Common/Misc.h"
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

	_PostFunctionRun();
}
// ------------------------------------ //
void Leviathan::QueuedTask::_PreFunctionRun(){

}

void Leviathan::QueuedTask::_PostFunctionRun(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::QueuedTask::CanBeRan(const QueuedTaskCheckValues* const checkvalues){
	return true;
}

DLLEXPORT bool Leviathan::QueuedTask::MustBeRanBefore(int eventtypeidentifier){
	return eventtypeidentifier == TASK_MUSTBERAN_BEFORE_EXIT;
}

DLLEXPORT bool Leviathan::QueuedTask::IsRepeating(){
	return false;
}
// ------------------ QueuedTaskCheckValues ------------------ //
Leviathan::QueuedTaskCheckValues::QueuedTaskCheckValues() : CurrentTime(Misc::GetThreadSafeSteadyTimePoint()){

}
// ------------------ ConditionalTask ------------------ //
DLLEXPORT Leviathan::ConditionalTask::ConditionalTask(boost::function<void ()> functorun, boost::function<bool ()> canberuncheck) :
	QueuedTask(functorun), TaskCheckingFunc(canberuncheck)
{

}

DLLEXPORT Leviathan::ConditionalTask::~ConditionalTask(){

}

DLLEXPORT bool Leviathan::ConditionalTask::CanBeRan(const QueuedTaskCheckValues* const checkvalues){
	return TaskCheckingFunc();
}
// ------------------ DelayedTask ------------------ //
DLLEXPORT Leviathan::DelayedTask::DelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &delaytime) : QueuedTask(functorun),
	ExecutionTime(Misc::GetThreadSafeSteadyTimePoint()+delaytime)
{

}

DLLEXPORT Leviathan::DelayedTask::DelayedTask(boost::function<void ()> functorun, const WantedClockType::time_point &executetime) :
	QueuedTask(functorun), ExecutionTime(executetime)
{

}

DLLEXPORT Leviathan::DelayedTask::~DelayedTask(){

}

DLLEXPORT bool Leviathan::DelayedTask::CanBeRan(const QueuedTaskCheckValues* const checkvalues){
	// Check is the current time past our timestamp //
	return checkvalues->CurrentTime >= ExecutionTime;
}
// ------------------ RepeatingDelayedTask ------------------ //
DLLEXPORT Leviathan::RepeatingDelayedTask::RepeatingDelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &bothdelays) :
	DelayedTask(functorun, bothdelays), ShouldRunAgain(true), TimeBetweenExecutions(bothdelays)
{

}

DLLEXPORT Leviathan::RepeatingDelayedTask::RepeatingDelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &initialdelay,
	const MicrosecondDuration &followingduration) : DelayedTask(functorun, initialdelay), ShouldRunAgain(true), TimeBetweenExecutions(followingduration)
{

}

DLLEXPORT Leviathan::RepeatingDelayedTask::~RepeatingDelayedTask(){

}

DLLEXPORT bool Leviathan::RepeatingDelayedTask::IsRepeating(){
	return ShouldRunAgain;
}

DLLEXPORT void Leviathan::RepeatingDelayedTask::SetRepeatStatus(bool newvalue){
	ShouldRunAgain = newvalue;
}

void Leviathan::RepeatingDelayedTask::_PostFunctionRun(){
	// Set new execution point in time //
	ExecutionTime = Misc::GetThreadSafeSteadyTimePoint()+TimeBetweenExecutions;
}
// ------------------ RunCountedDelayedTask ------------------ //
DLLEXPORT Leviathan::RepeatCountedDelayedTask::RepeatCountedDelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &bothdelays,
	int repeatcount) : DelayedTask(functorun, bothdelays), MaxRepeats(repeatcount), TimeBetweenExecutions(bothdelays), RepeatedCount(0)
{

}

DLLEXPORT Leviathan::RepeatCountedDelayedTask::RepeatCountedDelayedTask(boost::function<void ()> functorun, const MicrosecondDuration &initialdelay,
	const MicrosecondDuration &followingduration, int repeatcount) : DelayedTask(functorun, initialdelay), MaxRepeats(repeatcount),
	TimeBetweenExecutions(followingduration), RepeatedCount(0)
{

}

DLLEXPORT Leviathan::RepeatCountedDelayedTask::~RepeatCountedDelayedTask(){

}

DLLEXPORT bool Leviathan::RepeatCountedDelayedTask::IsRepeating(){
	// Increment count and see if there are still repeats left //
	return ++RepeatedCount < MaxRepeats;
}

DLLEXPORT void Leviathan::RepeatCountedDelayedTask::StopRepeating(){
	// This should do the trick //
	MaxRepeats = 0;
}

void Leviathan::RepeatCountedDelayedTask::_PostFunctionRun(){
	// Set new execution point in time //
	ExecutionTime = Misc::GetThreadSafeSteadyTimePoint()+TimeBetweenExecutions;
}

DLLEXPORT int Leviathan::RepeatCountedDelayedTask::GetRepeatCount() const{
	return RepeatedCount;
}

DLLEXPORT bool Leviathan::RepeatCountedDelayedTask::IsThisLastRepeat() const{
	return RepeatedCount+1 >= MaxRepeats;
}
