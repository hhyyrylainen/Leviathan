#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TIMER
#include "Timer.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

Timer::Timer(){
	// init values //
	Delta = 0;
	FramesSecond = 0;
	MaxframesSecond = 0;
	iFrameCount = 0;

	Fixeddeltatime = 0.0f;
	UseFixedDelta = false;

	LastTick64 = 0;

	QueryPerformanceFrequency((LARGE_INTEGER*)&TicksPerSecond64);
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTicks64);
	StartTimeTicks64 = CurrentTicks64;
	OneSecTicks64 = CurrentTicks64;

}
// ------------------------------------ //
void Timer::Update(){
	LastTick64 = CurrentTicks64;
	
	// update delta time //

	if(UseFixedDelta){
		Delta = Fixeddeltatime;
	} else {
		//Delta = (float)((__int64)CurrentTicks64 - (__int64)LastTick64) / (__int64)TicksPerSecond64;
		Delta = (float)(CurrentTicks64 - LastTick64) / TicksPerSecond64;
	}

	// count frame rate //
	if((float)(CurrentTicks64 - OneSecTicks64) / TicksPerSecond64 < 1.0f){
		// frame has passed //
		iFrameCount++;
	} else {
		FramesSecond = (float)iFrameCount;

		if(FramesSecond > MaxframesSecond)
			MaxframesSecond = (int)FramesSecond;

		iFrameCount = 0;
		OneSecTicks64 = CurrentTicks64;
	}

}
void Timer::Reset(){
	// reset some values //
	Delta = 0;
	FramesSecond = 0;
	iFrameCount = 0;
}
// ------------------------------------ //
int Timer::Framerate(){
	return (int)FramesSecond;
}

int Timer::MaxFramerate(){
	return MaxframesSecond;
}

int Timer::FrameCount(){
	return iFrameCount;
}

float Timer::RunTime(){
	return (float)(CurrentTicks64 - StartTimeTicks64)/TicksPerSecond64;
}

float Timer::Elapsed(){
	return Delta;
}
// ------------------------------------ //
void Timer::SetFixedStep(float step){
	if(step <= 0.0f){
		UseFixedDelta = false;
		Fixeddeltatime = 0.0f;
	} else {
		UseFixedDelta = true;
		Fixeddeltatime = step;
	}
}