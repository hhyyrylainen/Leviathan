#ifndef LEVIATHAN_TIMINGMONITOR
#define LEVIATHAN_TIMINGMONITOR
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //

#define TIMINGMONITOR_STYLE_RESULT_DEFAULT	100
#define TIMINGMONITOR_STYLE_RESULT_NONE		200

#define ADDTIMERFROSCOPE(x,y) Leviathan::ScopeTimer thisscopetimer_##x##(y);

#define ADDTIMEFORSCOPECALLER(x,y) ADDTIMERFROSCOPE(x, y)

#define QUICKTIME_THISSCOPE ADDTIMEFORSCOPECALLER(__LINE__, __WFUNCTION__)

#define BASETIMERNAME_FOR_SCROPE_TIMER	L"Scopetimer_for_id_:"

namespace Leviathan{
	struct TimingMonitorClock{
	public:
		TimingMonitorClock(const wstring& name, int style);
		int EndMonitoring();

		wstring Name;
		__int64 StartTime;
		__int64 EndTime;
		int CurrentElapsed;
		int Style;

	};



	// uninstantiable class aka "static" class //
	class TimingMonitor /*: public Object*/{
	public:
		DLLEXPORT static void StartTiming(const wstring& name, int style = TIMINGMONITOR_STYLE_RESULT_DEFAULT);
		DLLEXPORT static int GetCurrentElapsed(const wstring& name);
		DLLEXPORT static int StopTiming(const wstring& name, bool printoutput = true);

		DLLEXPORT static int GetCurrentTimerCount();

	private:
		TimingMonitor::TimingMonitor();
		TimingMonitor::TimingMonitor(const TimingMonitor& other);
		TimingMonitor::~TimingMonitor();

		// ---------------------- //
		static vector<shared_ptr<TimingMonitorClock>> Timers;
	};

	class ScopeTimer{
	public:
		DLLEXPORT ScopeTimer(const wstring& source);
		DLLEXPORT ~ScopeTimer();
	protected:
		wstring TimerName;
		wstring Source;

		static int CurID;
	};
}
#endif