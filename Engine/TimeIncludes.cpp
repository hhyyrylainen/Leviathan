// ------------------------------------ //
#include "TimeIncluides.h"
using namespace Leviathan;
// ------------------------------------ //
__int64 Time::GetTimeMs64(){
#ifdef _WIN32
    /* Windows */
	FILETIME ft;
	LARGE_INTEGER li;

    /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
     * to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	__int64 ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10000; /* From 100 nano seconds (10^-7) to 1 millisecond (10^-3) intervals */

	return ret;
#elif defined __linux__
    /* Linux */
	struct timeval tv;

	gettimeofday(&tv, NULL);

	__int64 ret = tv.tv_usec;
	/* Convert from micro seconds (10^-6) to milliseconds (10^-3) */
	ret /= 1000;

	/* Adds the seconds (10^0) after converting them to milliseconds (10^-3) */
	ret += (tv.tv_sec * 1000);

	return ret;
#else
#error no working get time on platform
#endif
}

__int64 Time::GetTimeMicro64(){
#ifdef _WIN32
    /* Windows */
	FILETIME ft;
	LARGE_INTEGER li;

    /* Get the amount of 100 nano seconds intervals elapsed since January 1, 1601 (UTC) and copy it
     * to a LARGE_INTEGER structure. */
	GetSystemTimeAsFileTime(&ft);
	li.LowPart = ft.dwLowDateTime;
	li.HighPart = ft.dwHighDateTime;

	__int64 ret = li.QuadPart;
	ret -= 116444736000000000LL; /* Convert from file time to UNIX epoch time. */
	ret /= 10; /* From 100 nano seconds (10^-7) to 1 microsecond (10^-6) intervals */

	return ret;
#elif defined __linux__
	/* Linux */
	struct timeval tv;

	gettimeofday(&tv, NULL);

	__int64 ret = tv.tv_usec;

	/* Adds the seconds (10^0) after converting them to microseconds (10^-6) */
	ret += (tv.tv_sec * 1000000);

	return ret;
#else
#error no working get time on platform
#endif
}
// ------------------------------------ //
DLLEXPORT WantedClockType::time_point Time::GetThreadSafeSteadyTimePoint(){
	// The Boost function may assert so we need to pass error object to it //
	boost::system::error_code timerror;

	auto result = boost::chrono::high_resolution_clock::now(timerror);

	// Check error code //
	if(timerror){
        wstring error = Convert::StringToWstring(timerror.message());
		// Probably caused an error //
		Logger::Get()->Warning(L"Misc: GetThreadSafeSteadyTimePoint: failed to get system time from Boost, error:"
            +error+L", recursing");
		// Let's fix this by recursing and causing a stack overflow
		return GetThreadSafeSteadyTimePoint();
	}

	return result;
}
// ------------------------------------ //

