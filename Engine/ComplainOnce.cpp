#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_COMPLAINONCE
#include "ComplainOnce.h"
#endif
using namespace Leviathan;
// ------------------------------------ //
// singleton class private constructors //
Leviathan::ComplainOnce::ComplainOnce(){

}

Leviathan::ComplainOnce::~ComplainOnce(){

}
// ------------------------------------ //
DLLEXPORT bool Leviathan::ComplainOnce::PrintWarningOnce(const wstring& warning, const wstring& message){
	// print only once to log //
	if(DataStore::Get()->AddValueIfDoesntExist(warning, 1)){
		// value wasn't there, print //
		Logger::Get()->Warning(message, false);
		return true;
	}
	return false;
}
// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


