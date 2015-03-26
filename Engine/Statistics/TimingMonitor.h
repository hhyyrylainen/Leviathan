#ifndef LEVIATHAN_TIMINGMONITOR
#define LEVIATHAN_TIMINGMONITOR
// ------------------------------------ //
// Reduce bloat in precompiled header
// ------------------------------------ //
// ---- includes ---- //
#include <memory>

#define TIMINGMONITOR_STYLE_RESULT_DEFAULT	100
#define TIMINGMONITOR_STYLE_RESULT_NONE		200

#ifdef _MSC_VER
#define ADDTIMERFROSCOPE(x,y) Leviathan::ScopeTimer thisscopetimer_##x##(y);
#else
#define ADDTIMERFROSCOPE(x,y) Leviathan::ScopeTimer thisscopetimer_(y);
#endif

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


	class TimingMonitor{
	public:
		DLLEXPORT static void StartTiming(const wstring& name, int style = TIMINGMONITOR_STYLE_RESULT_DEFAULT);
		DLLEXPORT static int GetCurrentElapsed(const wstring& name);
		DLLEXPORT static int StopTiming(const wstring& name, bool printoutput = true);

		DLLEXPORT static int GetCurrentTimerCount();
		DLLEXPORT static void ClearTimers();

	private:
		TimingMonitor();
		TimingMonitor(const TimingMonitor& other);
		~TimingMonitor();

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
