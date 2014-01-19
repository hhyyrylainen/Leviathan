#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_COMPLAINONCE
#include "ComplainOnce.h"
#endif
#include "Common/Misc.h"
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
	if(Misc::DoesVectorContainValuePointers<wstring>(FiredErrors, &warning)){
		// value wasn't there, print //
		Logger::Get()->Warning(message, false);
		// add //
		FiredErrors.push_back(new wstring(warning));
		return true;
	}
	return false;
}

DLLEXPORT  bool Leviathan::ComplainOnce::PrintErrorOnce(const wstring& error, const wstring& message){
	// print only once to log //
	if(Misc::DoesVectorContainValuePointers<wstring>(FiredErrors, &error)){
		// value wasn't there, print //
		Logger::Get()->Error(message, false);
		// add //
		FiredErrors.push_back(new wstring(error));
		return true;
	}
	return false;
}

vector<wstring*> Leviathan::ComplainOnce::FiredErrors;

// ------------------------------------ //

// ------------------------------------ //

// ------------------------------------ //


