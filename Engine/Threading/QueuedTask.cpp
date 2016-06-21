// ------------------------------------ //
#include "QueuedTask.h"

using namespace Leviathan;
// ------------------------------------ //
DLLEXPORT Leviathan::QueuedTask::QueuedTask(std::function<void ()> functorun) :
    FunctionToRun(functorun)
{

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
Leviathan::QueuedTaskCheckValues::QueuedTaskCheckValues() :
    CurrentTime(Time::GetThreadSafeSteadyTimePoint())
{

}
// ------------------ ConditionalTask ------------------ //
DLLEXPORT Leviathan::ConditionalTask::ConditionalTask(
    std::function<void ()> functorun, std::function<bool ()> canberuncheck) :
	QueuedTask(functorun), TaskCheckingFunc(canberuncheck)
{

}

DLLEXPORT Leviathan::ConditionalTask::~ConditionalTask(){

}

DLLEXPORT bool Leviathan::ConditionalTask::CanBeRan(
    const QueuedTaskCheckValues* const checkvalues)
{
	return TaskCheckingFunc();
}
// ------------------ ConditionalDelayedTask ------------------ //
DLLEXPORT Leviathan::ConditionalDelayedTask::ConditionalDelayedTask(
    std::function<void ()> functorun, std::function<bool ()> canberuncheck, 
	const MicrosecondDuration &delaytime) :
    QueuedTask(functorun), TaskCheckingFunc(canberuncheck), 
	CheckingTime(Time::GetThreadSafeSteadyTimePoint()+delaytime), DelayBetweenChecks(delaytime)
{

}

DLLEXPORT Leviathan::ConditionalDelayedTask::~ConditionalDelayedTask(){

}

DLLEXPORT bool Leviathan::ConditionalDelayedTask::CanBeRan(
    const QueuedTaskCheckValues* const checkvalues)
{
	// Check is it too early //
	if(checkvalues->CurrentTime < CheckingTime)
		return false;

	// Adjust the next check time //
	CheckingTime = Time::GetThreadSafeSteadyTimePoint()+DelayBetweenChecks;

	// Run the checking function //
	return TaskCheckingFunc();
}
// ------------------ DelayedTask ------------------ //
DLLEXPORT Leviathan::DelayedTask::DelayedTask(std::function<void ()> functorun,
    const MicrosecondDuration &delaytime) : QueuedTask(functorun),
	ExecutionTime(Time::GetThreadSafeSteadyTimePoint()+delaytime)
{

}

DLLEXPORT Leviathan::DelayedTask::DelayedTask(std::function<void ()> functorun,
    const WantedClockType::time_point &executetime) :
	QueuedTask(functorun), ExecutionTime(executetime)
{

}

DLLEXPORT Leviathan::DelayedTask::~DelayedTask(){

}

DLLEXPORT bool Leviathan::DelayedTask::CanBeRan(
    const QueuedTaskCheckValues* const checkvalues)
{
	// Check is the current time past our timestamp //
	return checkvalues->CurrentTime >= ExecutionTime;
}
// ------------------ RepeatingDelayedTask ------------------ //
DLLEXPORT Leviathan::RepeatingDelayedTask::RepeatingDelayedTask(
    std::function<void ()> functorun, const MicrosecondDuration &bothdelays) :
	DelayedTask(functorun, bothdelays), TimeBetweenExecutions(bothdelays), ShouldRunAgain(true)
{

}

DLLEXPORT Leviathan::RepeatingDelayedTask::RepeatingDelayedTask(
    std::function<void ()> functorun, const MicrosecondDuration &initialdelay,
	const MicrosecondDuration &followingduration) :
    DelayedTask(functorun, initialdelay), TimeBetweenExecutions(followingduration),
    ShouldRunAgain(true)
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
	ExecutionTime = Time::GetThreadSafeSteadyTimePoint()+TimeBetweenExecutions;
}
// ------------------ RepeatCountedTask ------------------ //
DLLEXPORT Leviathan::RepeatCountedTask::RepeatCountedTask(std::function<void ()> functorun,
    size_t repeatcount) :
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

DLLEXPORT size_t Leviathan::RepeatCountedTask::GetRepeatCount() const
{
	return RepeatedCount;
}

DLLEXPORT bool Leviathan::RepeatCountedTask::IsThisLastRepeat() const{
	return RepeatedCount + 1 >= MaxRepeats;
}
// ------------------ RepeatCountedDelayedTask ------------------ //
DLLEXPORT Leviathan::RepeatCountedDelayedTask::RepeatCountedDelayedTask(
    std::function<void ()> functorun,
    const MicrosecondDuration &bothdelays, int repeatcount) :
    RepeatCountedTask(functorun, repeatcount), TimeBetweenExecutions(bothdelays),
    ExecutionTime(Time::GetThreadSafeSteadyTimePoint() + bothdelays)
{

}

DLLEXPORT Leviathan::RepeatCountedDelayedTask::RepeatCountedDelayedTask(
    std::function<void ()> functorun,
    const MicrosecondDuration &initialdelay,
	const MicrosecondDuration &followingduration, int repeatcount) :
    RepeatCountedTask(functorun, repeatcount), TimeBetweenExecutions(followingduration),
    ExecutionTime(Time::GetThreadSafeSteadyTimePoint()+initialdelay)
{

}

DLLEXPORT Leviathan::RepeatCountedDelayedTask::~RepeatCountedDelayedTask(){

}

DLLEXPORT bool Leviathan::RepeatCountedDelayedTask::CanBeRan(
    const QueuedTaskCheckValues* const checkvalues)
{
	// Check is the current time past our timestamp //
	return checkvalues->CurrentTime >= ExecutionTime;
}

void Leviathan::RepeatCountedDelayedTask::_PostFunctionRun(){
	// Set new execution point in time //
	ExecutionTime = Time::GetThreadSafeSteadyTimePoint()+TimeBetweenExecutions;
}



