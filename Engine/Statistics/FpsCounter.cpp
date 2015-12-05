// ------------------------------------ //
#include "FpsCounter.h"
using namespace Leviathan;
// ------------------------------------ //
FpsCounter::FpsCounter() :
    passedtime(0), MaxFps(0), MinFps(0), Fps(0), Framecount(0), SinceLast(0)
{

}
// ------------------------------------ //
void FpsCounter::Frame(int timepassed){

	Framecount++;
	SinceLast = 0;

	if(passedtime < 0)
		passedtime = 0;
    
	passedtime += timepassed;
    
	if(passedtime > 6000){
		passedtime = passedtime-((passedtime/1000)*1000);

		Fps = Framecount;
		if(Fps > MaxFps)
			MaxFps = Fps;
		if(Fps < MinFps)
			MinFps = Fps;


		Framecount = 0;
		return;
	}

	if(passedtime > 1000){
		passedtime -= 1000;

		Fps = Framecount;
		if(Fps > MaxFps)
			MaxFps = Fps;
		if(Fps < MinFps)
			MinFps = Fps;


		Framecount = 0;
	}
}
// ------------------------------------ //
int FpsCounter::GetFps(){
	return Fps;
}
// ------------------------------------ //
bool FpsCounter::ShouldRender(int passed, int maxfps){
	// calculate how many frames should be rendered
	int timeperframe = (int)((1000.f/maxfps)+0.5f);

    // how many frames should have been rendered
	int nowrend = (int)((passedtime/1000.f)*maxfps+0.5f);


	if(nowrend > Framecount){
		// falling behind //

		SinceLast += nowrend-(Framecount);

	} else if(nowrend < Framecount){
		// getting ahead //

		SinceLast += nowrend-(Framecount+1);

	}


	if(SinceLast >= timeperframe)
		return true;
    
	return false;
}

void FpsCounter::FakeFrame(int mspassed){

	if(SinceLast < 0)
		SinceLast = 0;
    
	SinceLast += mspassed;


	if(passedtime < 0)
		passedtime = 0;
    
	passedtime += mspassed;
    
	if(passedtime > 6000){
        
		passedtime = passedtime-((passedtime/1000)*1000);

		Fps = Framecount;
        
		if(Fps > MaxFps)
			MaxFps = Fps;
        
		if(Fps < MinFps)
			MinFps = Fps;

		Framecount = 0;
		return;
	}
    
	if(passedtime > 1000){
        
		passedtime -= 1000;

		Fps = Framecount;
        
		if(Fps > MaxFps)
			MaxFps = Fps;
        
		if(Fps < MinFps)
			MinFps = Fps;

		Framecount = 0;
	}
}
