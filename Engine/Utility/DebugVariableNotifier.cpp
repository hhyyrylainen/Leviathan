// ------------------------------------ //
#include "DebugVariableNotifier.h"
using namespace Leviathan;
using namespace std;
// ------------------------------------ //

map<std::string, std::shared_ptr<VariableBlock>> Leviathan::DebugVariableNotifier::CapturedValues;
// ------------------------------------ //
DLLEXPORT void Leviathan::DebugVariableNotifier::UpdateVariable(const std::string &name,
    VariableBlock* valueofvariable)
{
	CapturedValues[name] = std::shared_ptr<VariableBlock>(valueofvariable);
}
// ------------------------------------ //
DLLEXPORT void Leviathan::DebugVariableNotifier::PrintVariables(){
	// prefix value (note: we use Write to avoid having excessive [INFO] tags) //
	Logger::Get()->Write("// ------------------ DebugVariableNotifier ------------------ //");

	// loop variables and print them //
	for(auto iterator = CapturedValues.begin(); iterator != CapturedValues.end(); iterator++) {
		// convert variable to std::string for printing //
		string toprintvalue;
		iterator->second->ConvertAndAssingToVariable<string>(toprintvalue);

		// print data from format ->first is name and ->second is variable //
		Logger::Get()->Write("\""+iterator->first+"\": "+toprintvalue);
	}

	Logger::Get()->Write("// ------------------ PrintVariables ------------------ //");
}
// ------------------------------------ //






