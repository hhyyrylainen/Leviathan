#pragma once

//! \file Includes and definitions for time keeping
#include "Define.h"

#include <stdint.h>
#include <chrono>


// Standard type time durations //
typedef std::chrono::duration<__int64, std::milli> MillisecondDuration;
typedef std::chrono::duration<__int64, std::micro> MicrosecondDuration;
typedef std::chrono::duration<float, std::ratio<1>> SecondDuration;

typedef std::chrono::high_resolution_clock WantedClockType;


namespace Leviathan{

    class Time{
    public:

        DLLEXPORT static int64_t GetTimeMs64();
		DLLEXPORT static int64_t GetTimeMicro64();
        
		//! \brief Gets the current time in a thread safe way
		//!
		//! This should always be used when getting the time to avoid segfaulting
		DLLEXPORT static WantedClockType::time_point GetThreadSafeSteadyTimePoint();
    };
}


