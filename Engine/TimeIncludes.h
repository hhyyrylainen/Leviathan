// Leviathan Game Engine
// Copyright (c) 2012-2018 Henri Hyyryläinen
#pragma once

//! \file Includes and definitions for time keeping
#include "Include.h"

#include <chrono>
#include <cstdint>


// Standard type time durations //
typedef std::chrono::duration<int64_t, std::milli> MillisecondDuration;
typedef std::chrono::duration<int64_t, std::micro> MicrosecondDuration;
typedef std::chrono::duration<float, std::ratio<1>> SecondDuration;

typedef std::chrono::high_resolution_clock WantedClockType;


namespace Leviathan {

class Time {
public:
    DLLEXPORT static int64_t GetTimeMs64();
    DLLEXPORT static int64_t GetTimeMicro64();

    //! \note This should be not required when using the standard
    DLLEXPORT static WantedClockType::time_point GetThreadSafeSteadyTimePoint();
};
} // namespace Leviathan
