// ------------------------------------ //
#include "TimeIncludes.h"

#ifdef __linux__
#include <sys/time.h>
#elif _WIN32
#include "WindowsInclude.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
int64_t Time::GetTimeMs64()
{
#ifdef _WIN32
    /* Windows */
    FILETIME ft;
    LARGE_INTEGER li;

    /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and
     * copy it to a LARGE_INTEGER structure. */
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    int64_t ret = li.QuadPart;
    ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
    ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

    return ret;
#elif defined __linux__
    /* Linux */
    struct timeval tv;

    gettimeofday(&tv, NULL);

    int64_t ret = tv.tv_usec;
    /* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
    ret /= 1000;

    /* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
    ret += (tv.tv_sec * 1000);

    return ret;
#else
#error no working get time on platform
#endif
}

int64_t Time::GetTimeMicro64()
{
#ifdef _WIN32
    /* Windows */
    FILETIME ft;
    LARGE_INTEGER li;

    /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and
     * copy it to a LARGE_INTEGER structure. */
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    int64_t ret = li.QuadPart;
    ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
    ret /= 10; /* From 100 nano seconds (10^-7) to 1 microsecond (10^-6) intervals */

    return ret;
#elif defined __linux__
    /* Linux */
    struct timeval tv;

    gettimeofday(&tv, NULL);

    int64_t ret = tv.tv_usec;

    /* Adds the seconds (10^0) after converting them to microseconds (10^-6) */
    ret += (tv.tv_sec * 1000000);

    return ret;
#else
#error no working get time on platform
#endif
}
// ------------------------------------ //
DLLEXPORT WantedClockType::time_point Time::GetThreadSafeSteadyTimePoint()
{
    return WantedClockType::now();
}
// ------------------------------------ //
