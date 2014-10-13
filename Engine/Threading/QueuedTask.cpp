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
// ------------------ ConditionalDelayedTask ------------------ //
DLLEXPORT Leviathan::ConditionalDelayedTask::ConditionalDelayedTask(boost::function<void ()> functorun, boost::function<bool ()> canberuncheck, 
	const MicrosecondDuration &delaytime) : QueuedTask(functorun), TaskCheckingFunc(canberuncheck), 
	CheckingTime(Misc::GetThreadSafeSteadyTimePoint()+delaytime), DelayBetweenChecks(delaytime)
{

}

DLLEXPORT Leviathan::ConditionalDelayedTask::~ConditionalDelayedTask(){

}

DLLEXPORT bool Leviathan::ConditionalDelayedTask::CanBeRan(const QueuedTaskCheckValues* const checkvalues){
	// Check is it too early //
	if(checkvalues->CurrentTime < CheckingTime)
		return false;

	// Adjust the next check time //
	CheckingTime = Misc::GetThreadSafeSteadyTimePoint()+DelayBetweenChecks;

	// Run the checking function //
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
// ------------------ RepeatCountedTask ------------------ //
DLLEXPORT Leviathan::RepeatCountedTask::RepeatCountedTask(boost::function<void ()> functorun, int repeatcount) :
    QueuedTask(functorun), MaxRepeats(repeatcount), RepeatedCount(0)
{

}

DLLEXPORT Leviathan::RepeatCountedTask::~RepeatCountedTask(){

}

DLLEXPORT bool Leviathan::RepeatCountedTask::IsRepeating(){
	// Increment count and see if there are still repeats left //
	return ++RepeatedCount < MaxRepeats;
}

DLLEXPORT void Leviathan::RepeatCountedTask::StopRepeating(){
	// This should do the trick //
	MaxRepeats = 0;
}

DLLEXPORT int Leviathan::RepeatCountedTask::GetRepeatCount() const{
	return RepeatedCount;
}

DLLEXPORT bool Leviathan::RepeatCountedTask::IsThisLastRepeat() const{
	return RepeatedCount+1 >= MaxRepeats;
}
// ------------------ RepeatCountedDelayedTask ------------------ //
DLLEXPORT Leviathan::RepeatCountedDelayedTask::RepeatCountedDelayedTask(boost::function<void ()> functorun,
    const MicrosecondDuration &bothdelays, int repeatcount) :
    DelayedTask(functorun, bothdelays), MaxRepeats(repeatcount),  TimeBetweenExecutions(bothdelays), RepeatedCount(0),
    ExecutionTime(Misc::GetThreadSafeSteadyTimePoint()+bothdelays)
{

}

DLLEXPORT Leviathan::RepeatCountedDelayedTask::RepeatCountedDelayedTask(boost::function<void ()> functorun,
    const MicrosecondDuration &initialdelay,
	const MicrosecondDuration &followingduration, int repeatcount) :
    DelayedTask(functorun, initialdelay), MaxRepeats(repeatcount), TimeBetweenExecutions(followingduration),
    RepeatedCount(0), ExecutionTime(Misc::GetThreadSafeSteadyTimePoint()+initialdelay)
{

}

DLLEXPORT Leviathan::RepeatCountedDelayedTask::~RepeatCountedDelayedTask(){

}

DLLEXPORT bool Leviathan::RepeatCountedDelayedTask::CanBeRan(const QueuedTaskCheckValues* const checkvalues){
	// Check is the current time past our timestamp //
	return checkvalues->CurrentTime >= ExecutionTime;
}

void Leviathan::RepeatCountedDelayedTask::_PostFunctionRun(){
	// Set new execution point in time //
	ExecutionTime = Misc::GetThreadSafeSteadyTimePoint()+TimeBetweenExecutions;
}



