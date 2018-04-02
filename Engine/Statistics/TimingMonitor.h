#pragma once
// ------------------------------------ //
#include "Include.h"
#include <memory>
#include <cstdint>
#include <string>
#include <vector>

#define TIMINGMONITOR_STYLE_RESULT_DEFAULT	100
#define TIMINGMONITOR_STYLE_RESULT_NONE		200

#ifdef _MSC_VER
#define ADDTIMERFROSCOPE(x,y) Leviathan::ScopeTimer thisscopetimer_##x##(y);
#else
#define ADDTIMERFROSCOPE(x,y) Leviathan::ScopeTimer thisscopetimer_(y);
#endif

#define ADDTIMEFORSCOPECALLER(x,y) ADDTIMERFROSCOPE(x, y)

#define QUICKTIME_THISSCOPE ADDTIMEFORSCOPECALLER(__LINE__, __FUNCTION__)

#define BASETIMERNAME_FOR_SCROPE_TIMER	"Scopetimer_for_id_:"

namespace Leviathan{
    struct TimingMonitorClock{
    public:
        TimingMonitorClock(const std::string& name, int style);
        int EndMonitoring();

        std::string Name;
        int64_t StartTime;
        int64_t EndTime;
        int CurrentElapsed;
        int Style;

    };


    class TimingMonitor{
    public:
        DLLEXPORT static void StartTiming(const std::string &name,
            int style = TIMINGMONITOR_STYLE_RESULT_DEFAULT);
        DLLEXPORT static int GetCurrentElapsed(const std::string &name);
        DLLEXPORT static int StopTiming(const std::string &name, bool printoutput = true);

        DLLEXPORT static size_t GetCurrentTimerCount();
        DLLEXPORT static void ClearTimers();

    private:
        TimingMonitor();
        TimingMonitor(const TimingMonitor& other);
        ~TimingMonitor();

        // ---------------------- //
        static std::vector<std::shared_ptr<TimingMonitorClock>> Timers;
    };

    class ScopeTimer{
    public:
        DLLEXPORT ScopeTimer(const std::string &source);
        DLLEXPORT ~ScopeTimer();
    protected:
        std::string TimerName;
        std::string Source;

        static int CurID;
    };
}

#ifdef LEAK_INTO_GLOBAL
using Leviathan::TimingMonitor;
using Leviathan::ScopeTimer;
#endif

