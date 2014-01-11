#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_MISC
#include "Misc.h"
#include "Define.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
#include "Logger.h"
#include "Utility/Convert.h"
#include "Common/DataStoring/DataBlock.h"
#include "Statistics/TimingMonitor.h"

#ifdef __linux__
#include <pthread.h>
#include <signal.h>
#endif

// string conversions
wstring Misc::EmptyString = L"";

// Can't remember from where these methods are loaned...
__int64 Misc::GetTimeMs64()
{
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
__int64 Misc::GetTimeMicro64()
{
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



DLLEXPORT void Leviathan::Misc::KillThread(boost::thread &threadtokill){
#ifdef _WIN32

	TerminateThread(threadtokill.native_handle(), 0);

#elif defined __linux__

	//pthread_kill(threadtokill.native_handle(), SIGIO);
	//pthread_kill(threadtokill.native_handle(), SIGUSR1);
	// This should work //
	pthread_cancel(threadtokill.native_handle());

#else
#error no working kill thread on platform!
#endif
}

bool Misc::CompareDataBlockTypeToTHISNameCheck(int datablock, int typenamecheckresult){
	if(typenamecheckresult == 0){
		// int //
		if(datablock == DATABLOCK_TYPE_INT)
			return true;
		return false;
	}
	if(typenamecheckresult == 1){
		// float //
		if(datablock == DATABLOCK_TYPE_FLOAT)
			return true;
		return false;
	}
	if(typenamecheckresult == 3){
		// bool //
		if(datablock == DATABLOCK_TYPE_BOOL)
			return true;
		return false;
	}
	if(typenamecheckresult == 4){
		// wstring //
		if(datablock == DATABLOCK_TYPE_WSTRING)
			return true;
		return false;
	}
	if(typenamecheckresult == 5){
		// void //
		if(datablock == DATABLOCK_TYPE_VOIDPTR)
			return true;
		return false;
	}

	return false;
}


wstring Misc::Errstring = L"ERROR";
std::string Leviathan::Misc::Errstrings = "ERROR";





