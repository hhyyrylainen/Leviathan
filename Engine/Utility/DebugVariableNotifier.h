#ifndef LEVIATHAN_DEBUGVARIABLENOTIFIER
#define LEVIATHAN_DEBUGVARIABLENOTIFIER
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common/DataStoring/DataBlock.h"

namespace Leviathan{

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