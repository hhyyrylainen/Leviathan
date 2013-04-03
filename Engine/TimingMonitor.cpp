#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_TIMINGMONITOR
#include "TimingMonitor.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
void Leviathan::TimingMonitor::StartTiming(const wstring& name, int style /*= TIMINGMONITOR_STYLE_RESULT_DEFAULT*/){
	Timers.push_back(shared_ptr<TimingMonitorClock>(new TimingMonitorClock(name, style)));
}

int Leviathan::TimingMonitor::GetCurrentElapsed(const wstring& name){
	// loop through timers and get right based on name //
	for(unsigned int i = 0; i < Timers.size(); i++){
		if(Timers[i]->Name == name){
			// "end" the timer and return it's result which is the elapsed time //
			return Timers[i]->EndMonitoring();
		}
	}
	return -1;
}

int Leviathan::TimingMonitor::StopTiming(const wstring& name, bool printoutput /*= true*/){
	// loop through timers and get right based on name //
	for(unsigned int i = 0; i < Timers.size(); i++){
		if(Timers[i]->Name == name){
			// "end" the timer and return it's result which is the elapsed time //
			// if specified print to log //
			int time = Timers[i]->EndMonitoring();
			if(printoutput){
				Logger::Get()->Info(L"TimingMonitor: Timer \""+name+L"\" stopped elapsed "+Convert::IntToWstring(time)+L" micro seconds ("+
					Convert::FloatToWstring(time/1000000.f)+L" s)", false);
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
int Leviathan::TimingMonitor::GetCurrentTimerCount(){
	return Timers.size();
}
// ------------------------------------ //
Leviathan::TimingMonitor::TimingMonitor(){

}

Leviathan::TimingMonitor::TimingMonitor(const TimingMonitor& other){

}

Leviathan::TimingMonitor::~TimingMonitor(){

}
// ------------------------------------ //
vector<shared_ptr<TimingMonitorClock>> Leviathan::TimingMonitor::Timers;
// ---------------- TimingMonitorClock -------------------- //
int Leviathan::TimingMonitorClock::EndMonitoring(){
	// get end time and calculate duration //
	EndTime = Misc::GetTimeMicro64();
	CurrentElapsed = (int)(EndTime-StartTime);
	return CurrentElapsed;
}

Leviathan::TimingMonitorClock::TimingMonitorClock(const wstring& name, int style){
	EndTime = -1;
	Name = name;

	CurrentElapsed = 0;
	StartTime = Misc::GetTimeMicro64();
	Style = style;
}
