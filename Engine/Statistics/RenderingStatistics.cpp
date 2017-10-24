// ------------------------------------ //
#include "RenderingStatistics.h"

#include "../Common/DataStoring/DataStore.h"
#include "../TimeIncludes.h"
using namespace Leviathan;
// ------------------------------------ //
Leviathan::RenderingStatistics::RenderingStatistics() : LastMinuteFPS(80), LastMinuteRenderTimes(320){

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

    LastMinuteFPSPos = 0;
    LastMinuteRenderTimesPos = 0;



    IsFirstFrame = true;
    EraseOld = true;
}

Leviathan::RenderingStatistics::~RenderingStatistics(){

}
// ------------------------------------ //
void Leviathan::RenderingStatistics::RenderingStart(){
    RenderingStartTime = Time::GetTimeMicro64();

    Frames++;
}

void Leviathan::RenderingStatistics::RenderingEnd(){
    RenderingEndTime = Time::GetTimeMicro64();

    RenderMCRSeconds = (int)(RenderingEndTime-RenderingStartTime);


    MakeSureHasEnoughRoom(LastMinuteRenderTimes, LastMinuteRenderTimesPos);

    LastMinuteRenderTimes[LastMinuteRenderTimesPos] = RenderMCRSeconds;
    ++LastMinuteRenderTimesPos;

    // half minute check //
    if(RenderingEndTime > HalfMinuteStartTime+(1000000*30)){

        HalfMinuteStartTime = RenderingEndTime;
        HalfMinuteMark();
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

    // Calculate the averages //
    int fpses = 0;

    for(size_t i = 0; i < LastMinuteFPSPos+1; i++){
        fpses += LastMinuteFPS[i];
    }

    if(LastMinuteFPSPos == 0){

        AverageFps = 0;

    } else {

        AverageFps = static_cast<int>(fpses/LastMinuteFPSPos+1);
    }

    // Frame time averages //
    int frametimes = 0;


    for(size_t i = 0; i < LastMinuteRenderTimesPos+1; i++){
        
        frametimes += LastMinuteRenderTimes[i];
    }

    if(LastMinuteRenderTimesPos == 0){
        
        AverageRenderTime = 0;
    } else {
        
        AverageRenderTime = static_cast<int>(frametimes/LastMinuteRenderTimesPos+1);
    }

    // Reset the insert positions //
    LastMinuteRenderTimesPos = 0;
    LastMinuteFPSPos = 0;
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
    
    MakeSureHasEnoughRoom(LastMinuteFPS, LastMinuteFPSPos);

    LastMinuteFPS[LastMinuteFPSPos] = FPS;
    ++LastMinuteFPSPos;

    EraseOld = false;
}
// ------------------------------------ //
bool Leviathan::RenderingStatistics::CanRenderNow(int maxfps, int& TimeSinceLastFrame){


    // calculate can a frame be rendered now without going over max fps //
    auto CurrentTime = Time::GetTimeMicro64();

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

    
    // Check has a second passed //
    bool Set = false;
    while(TimeFromLastSecond > 1000000){
        TimeFromLastSecond -= 1000000;
        Set = true;

    }

    if(Set){

        // second passed //
        SecondStartTime = CurrentTime;
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
    if(Frames <= FramesShouldBe){


        return true;
    }


    return false;
}
// ------------------------------------ //
void RenderingStatistics::MakeSureHasEnoughRoom(std::vector<int> &tarvec,
    const size_t &accessspot)
{
            
    if(tarvec.size() <= accessspot+1){
        if(tarvec.size() > 5000){

            tarvec.resize((size_t)(tarvec.size()*1.35f));
            Logger::Get()->Warning("RenderingStatistics: large frame time tracking buffer is "
                "getting larger, size: "+Convert::ToString(tarvec.size()));

        } else {

            tarvec.resize((size_t)(tarvec.size()*1.8f));
        }
    }
}








