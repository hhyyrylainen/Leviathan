// ------------------------------------ //
#include "TimingMonitor.h"

#include "Define.h"
#include "../TimeIncludes.h"
#include "../Utility/Convert.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //
void Leviathan::TimingMonitor::StartTiming(const std::string& name,
    int style /*= TIMINGMONITOR_STYLE_RESULT_DEFAULT*/)
{
    Timers.push_back(shared_ptr<TimingMonitorClock>(new TimingMonitorClock(name, style)));
}

int Leviathan::TimingMonitor::GetCurrentElapsed(const std::string& name){
    // loop through timers and get right based on name //
    for(unsigned int i = 0; i < Timers.size(); i++){
        if(Timers[i]->Name == name){
            // "end" the timer and return it's result which is the elapsed time //
            return Timers[i]->EndMonitoring();
        }
    }
    return -1;
}

int Leviathan::TimingMonitor::StopTiming(const std::string& name, bool printoutput /*= true*/){
    
    // loop through timers and get right based on name //
    for(size_t i = 0; i < Timers.size(); i++){
        if(Timers[i]->Name == name){
            // "end" the timer and return it's result which is the elapsed time //
            // if specified print to log //
            int time = Timers[i]->EndMonitoring();

            if(printoutput){
                Logger::Get()->Info("TimingMonitor: Timer \""+name+
                    "\" stopped elapsed "+Convert::ToString(time/1000000.f)+" s ("+
                    Convert::ToString(time)+" micro seconds)");
            }

            // remove the timer because memory might be filled with stuff //
            // smart pointer //
            Timers.erase(Timers.begin()+i);

            return time;
        }
    }
    
    return -1;
}
// ------------------------------------ //
size_t Leviathan::TimingMonitor::GetCurrentTimerCount(){
    return Timers.size();
}
// ------------------------------------ //
Leviathan::TimingMonitor::TimingMonitor(){

}

Leviathan::TimingMonitor::~TimingMonitor(){

}

DLLEXPORT void Leviathan::TimingMonitor::ClearTimers(){

    for(size_t i = 0; i < Timers.size(); i++){
        if(i == 0)
            Logger::Get()->Info("TimingMonitor: leaked timers! names:");
        Logger::Get()->Write(Timers[i]->Name);
    }
    // just clear all timers vector and they will delete automatically //
    Timers.clear();
}

// ------------------------------------ //
vector<shared_ptr<TimingMonitorClock>> Leviathan::TimingMonitor::Timers;
// ---------------- TimingMonitorClock -------------------- //
int Leviathan::TimingMonitorClock::EndMonitoring(){
    // get end time and calculate duration //
    EndTime = Time::GetTimeMicro64();
    CurrentElapsed = (int)(EndTime-StartTime);
    return CurrentElapsed;
}

Leviathan::TimingMonitorClock::TimingMonitorClock(const std::string& name, int style){
    EndTime = -1;
    Name = name;

    CurrentElapsed = 0;
    StartTime = Time::GetTimeMicro64();
    Style = style;
}
// ---------------- ScopeTimer -------------------- //
DLLEXPORT Leviathan::ScopeTimer::ScopeTimer(const std::string& source){
    // create unique name for this timer //
    CurID++;
    TimerName = BASETIMERNAME_FOR_SCROPE_TIMER+Convert::ToString(CurID);
    Source = source;
    // start timer for this object //
    TimingMonitor::StartTiming(TimerName, TIMINGMONITOR_STYLE_RESULT_NONE);
}

DLLEXPORT Leviathan::ScopeTimer::~ScopeTimer(){
    // kill this timer //
    int ElapsedTime = TimingMonitor::StopTiming(TimerName, false);

    // print data //
    Logger::Get()->Info("ScopeTimer: "+Source+" Stopped elapsed: "+
        Convert::ToString(ElapsedTime/1000000.f)+" s ("+
        Convert::ToString(ElapsedTime)+" micro seconds)");
}

int Leviathan::ScopeTimer::CurID = 42;
