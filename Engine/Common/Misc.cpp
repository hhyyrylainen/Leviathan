#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_MISC
#include "Misc.h"
#include "Define.h"
#endif
#include "boost/filesystem.hpp"
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
DLLEXPORT void Leviathan::Misc::KillThread(boost::thread &threadtokill){
#ifdef _WIN32

	TerminateThread(threadtokill.native_handle(), 0);

#elif defined __linux__

	// This should work //
	int threadid = threadtokill.native_handle();
	
	if(threadid != 0){
		
		pthread_cancel(threadid);
	}

	threadtokill.detach();
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

// ------------------------------------ //
wstring Misc::Errstring = L"ERROR";
std::string Leviathan::Misc::Errstrings = "ERROR";
// ------------------------------------ //
DLLEXPORT std::string Leviathan::Misc::GetProcessDirectory(){

    return boost::filesystem::current_path().string();
}



