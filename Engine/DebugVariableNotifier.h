#ifndef LEVIATHAN_DEBUGVARIABLENOTIFIER
#define LEVIATHAN_DEBUGVARIABLENOTIFIER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "DataBlock.h"

namespace Leviathan{
	// struct that holds together everything related to a value //
	//struct DebugVariableStoredValue{
	//	// constructor to reduce code //
	//	inline DebugVariableStoredValue(const wstring &name, VariableBlock* variable) : Name(name), SVariable(variable){
	//	}
	//	inline DebugVariableStoredValue(const wstring &name, shared_ptr<VariableBlock> variable) : Name(name), SVariable(variable){
	//	}
	//	// name that appears in output //
	//	wstring Name;

	//	// actual value holder //
	//	shared_ptr<VariableBlock> SVariable;
	//};

	class DebugVariableNotifier : public Object{
	public:

		DLLEXPORT static void UpdateVariable(const wstring &name, VariableBlock* valueofvariable);

		// prints all values to log //
		DLLEXPORT static void PrintVariables();

	private:
		// singleton //
		inline DebugVariableNotifier(){}
		inline ~DebugVariableNotifier(){}

	protected:
		// stored variables //
		static map<wstring, shared_ptr<VariableBlock>> CapturedValues;

	};

}
#endif