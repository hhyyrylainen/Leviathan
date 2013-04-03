#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_RENDERINGSTATISTICS
#include "RenderingStatistics.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
Leviathan::RenderingStatistics::RenderingStatistics(){
	Frames = 0;

	FPS = 0;
	RenderMCRSeconds = 0;

	MinFPS = 9999;
	MaxFPS = 0;
	AverageFps = 0;

	MaxFrameTime = 0;
	MinFrameTime = 9999;
	AverageRenderTime = 0;

	HalfMinuteStartTime = 0;
	SecondStartTime = 0;
	RenderingStartTime = 0;
	RenderingEndTime = 0;

	DoubtfulCancel = 0;


	IsFirstFrame = true;
	EraseOld = true;
}

Leviathan::RenderingStatistics::~RenderingStatistics(){

}
// ------------------------------------ //
void Leviathan::RenderingStatistics::RenderingStart(){
	RenderingStartTime = Misc::GetTimeMicro64();

	Frames++;
}
void Leviathan::RenderingStatistics::RenderingEnd(){
	RenderingEndTime = Misc::GetTimeMicro64();

	RenderMCRSeconds = (int)(RenderingEndTime-RenderingStartTime);

	LastMinuteRenderTimes.push_back(RenderMCRSeconds);

	//// check is second passed //
	//if(RenderingEndTime > SecondStartTime+1000000){
	//	// second passed //
	//	SecondStartTime = RenderingEndTime;
	//	//DEBUG_OUTPUT(L"second pass\n");

	//	SecondMark();
	//}
	// half minute check //
	if(RenderingEndTime > HalfMinuteStartTime+(1000000*30)){

		HalfMinuteStartTime = RenderingEndTime;
		HalfMinuteMark();

		//DEBUG_OUTPUT(L"half pass\n");
	}

	if(IsFirstFrame)
		IsFirstFrame = false;

}
// ------------------------------------ //
void Leviathan::RenderingStatistics::ReportStats(DataStore* dstore){
	dstore->SetFPS(FPS);
	dstore->SetFPSAverage(AverageFps);
	dstore->SetFPSMax(MaxFPS);
	dstore->SetFPSMin(MinFPS);

	dstore->SetFrameTime(RenderMCRSeconds);
	dstore->SetFrameTimeAverage(AverageRenderTime);
	dstore->SetFrameTimeMax(MaxFrameTime);
	dstore->SetFrameTimeMin(MinFrameTime);
}

void Leviathan::RenderingStatistics::HalfMinuteMark(){
	EraseOld = true;

	// calculate averages //
	int fpses = 0;

	//// if first frame pop first values //
	//if(IsFirstFrame){
	//	IsFirstFrame = false;

	//	LastMinuteFPS.erase(LastMinuteFPS.begin());
	//	LastMinuteRenderTimes.erase(LastMinuteRenderTimes.begin());

	//	if((LastMinuteFPS.size() == 0) || (LastMinuteRenderTimes.size() == 0))
	//		return;
	//}

	for(unsigned int i = 0; i < LastMinuteFPS.size(); i++){
		fpses += LastMinuteFPS[i];
	}
	if(LastMinuteFPS.size() == 0){
		AverageFps = 0;
	} else {
		AverageFps = fpses/(int)LastMinuteFPS.size();
	}


	// frame time averages //
	int frametimes = 0;
	for(unsigned int i = 0; i < LastMinuteRenderTimes.size(); i++){
		frametimes += LastMinuteRenderTimes[i];
	}
	if(LastMinuteRenderTimes.size() == 0){
		AverageRenderTime = 0;
	} else {
		AverageRenderTime = frametimes/(int)LastMinuteRenderTimes.size();
	}

}

void Leviathan::RenderingStatistics::SecondMark(){
	FPS = Frames;
	Frames = 0;

	// checks on fps //
	if((FPS > MaxFPS) || (EraseOld)){
		MaxFPS = FPS;
	}
	if((FPS < MinFPS) || (EraseOld)){
		MinFPS = FPS;
	}
	// frame time checks //
	if((RenderMCRSeconds > MaxFrameTime) || (EraseOld)){
		MaxFrameTime = RenderMCRSeconds;
	}
	if((RenderMCRSeconds < MinFrameTime) || (EraseOld)){
		MinFrameTime = RenderMCRSeconds;
	}

	LastMinuteFPS.push_back(FPS);

	EraseOld = false;
}



// ------------------------------------ //
bool Leviathan::RenderingStatistics::CanRenderNow(int maxfps, int& TimeSinceLastFrame){


	// calculate can a frame be rendered now without going over max fps //
	__int64 CurrentTime = Misc::GetTimeMicro64();

	// check first frame //
	if(IsFirstFrame){
		// set values to current time //
		HalfMinuteStartTime = CurrentTime;
		SecondStartTime = CurrentTime;
		RenderingStartTime = CurrentTime;
		RenderingEndTime = CurrentTime;
		// on first frame always can render //
		return true;
	}

	int TimePassed = (int)(CurrentTime-RenderingStartTime);
	int TimeFromLastSecond = (int)(CurrentTime-SecondStartTime);

	if(TimeFromLastSecond < -1){
		// second passed //
		SecondStartTime = CurrentTime;
		//DEBUG_OUTPUT(L"second pass\n");
		TimeFromLastSecond = 0;

		SecondMark();
		return true;
	}

	//// check is second passed //
	//if((TimeFromLastSecond > 1000000) || (TimeFromLastSecond < -1)){
	//	// second passed //
	//	SecondStartTime = CurrentTime;
	//	//DEBUG_OUTPUT(L"second pass\n");
	//	TimeFromLastSecond = 0;

	//	SecondMark();
	//}
	// check has a second passed //
	bool Set = false;
	while(TimeFromLastSecond > 1000000){
		TimeFromLastSecond -= 1000000;
		Set = true;

	}

	if(Set){
		// second passed //
		SecondStartTime = CurrentTime;
		//DEBUG_OUTPUT(L"second pass\n");
				SecondMark();
	}

	// send back time before last frame in microseconds //
	TimeSinceLastFrame = TimePassed;


	// calculate how many frames should have been rendered //
	// divide current passed by 1 second and multiply frame rate to see how many should be at this time //
	float Percentage = (TimeFromLastSecond/1000000.f);
	int FramesShouldBe = (int)((Percentage*maxfps)+0.5f);

	// check do we have enough frames, if we don't we can render 1 //
	// or if a second has passed render then //
	if((Frames < FramesShouldBe)){
		//// check here that are there too many frames rendered, if there are, skip //
		//int CurrAverage = 0;
		//int fpses = 0;
		//for(unsigned int i = 0; i < LastMinuteFPS.size(); i++){
		//	fpses += LastMinuteFPS[i];
		//}
		//CurrAverage = fpses/(int)LastMinuteFPS.size();

		//if(CurrAverage > maxfps+3){
		//	// average might go over max fps, do emergency cancel render //
		//	DoubtfulCancel++;

		//	if(DoubtfulCancel > 4){
		//		DoubtfulCancel = 0;
		//		return false;
		//	}
		//}


		return true;
	}

	//// rendering might fail, just check the average before making the call //
	//int CurrAverage = 0;
	//int fpses = 0;
	//for(unsigned int i = 0; i < LastMinuteFPS.size(); i++){
	//	fpses += LastMinuteFPS[i];
	//}
	//CurrAverage = fpses/(int)LastMinuteFPS.size();

	//if((CurrAverage < maxfps)){
	//	// average might fall below max fps, do emergency render //
	//	return true;
	//}


	return false;
}
// ------------------------------------ //








