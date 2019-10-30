// Leviathan Game Engine
// Copyright (c) 2012-2019 Henri Hyyryläinen
//! \file Includes and definitions for time keeping
#pragma once

#include "Include.h"

#include <chrono>
#include <cstdint>

namespace Leviathan {

// Standard type time durations //
using MillisecondDuration = std::chrono::duration<int64_t, std::milli>;
using MicrosecondDuration = std::chrono::duration<int64_t, std::micro>;
using SecondDuration = std::chrono::duration<float, std::ratio<1>>;
using PreciseSecondDuration = std::chrono::duration<double, std::ratio<1>>;

using WantedClockType = std::chrono::high_resolution_clock;
using TimePoint = WantedClockType::time_point;

//! \brief Helper class for getting the time
class Time {
public:
    //! \todo Use of this should be phased out in favour of GetCurrentTimePoint
    DLLEXPORT static int64_t GetTimeMs64();
    DLLEXPORT static int64_t GetTimeMicro64();

    //! \note This should be not required when using the standard
    //! \todo If this is too slow for some uses add a variant that uses a less accurate clock
    DLLEXPORT static TimePoint GetCurrentTimePoint();
};
} // namespace Leviathan
