#include "Include.h"
// ------------------------------------ //
#ifndef LEVIATHAN_DEBUGVARIABLENOTIFIER
#include "DebugVariableNotifier.h"
#endif
using namespace Leviathan;
// ------------------------------------ //

map<wstring, shared_ptr<VariableBlock>> Leviathan::DebugVariableNotifier::CapturedValues;
// ------------------------------------ //
DLLEXPORT void Leviathan::DebugVariableNotifier::UpdateVariable(const wstring &name, VariableBlock* valueofvariable){
	CapturedValues[name] = shared_ptr<VariableBlock>(valueofvariable);
}
// ------------------------------------ //

// ------------------------------------ //
DLLEXPORT void Leviathan::DebugVariableNotifier::PrintVariables(){
	// prefix value (note: we use Write to avoid having excessive [INFO] tags) //
	Logger::Get()->Write(L"// ------------------ DebugVariableNotifier ------------------ //");

	// loop variables and print them //
	for(auto iterator = CapturedValues.begin(); iterator != CapturedValues.end(); iterator++) {
		// convert variable to wstring for printing //
		wstring toprintvalue;
		iterator->second->ConvertAndAssingToVariable<wstring>(toprintvalue);

		// print data from format ->first is name and ->second is variable //
		Logger::Get()->Write(L"\""+iterator->first+L"\": "+toprintvalue);
	}

	Logger::Get()->Write(L"// ------------------ PrintVariables ------------------ //", true);
}
// ------------------------------------ //






