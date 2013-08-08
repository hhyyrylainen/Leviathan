#ifndef LEVIATHAN_SCRIPTRUNNINGSETUP
#define LEVIATHAN_SCRIPTRUNNINGSETUP
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "DataBlock.h"


namespace Leviathan{

	enum SCRIPT_RUNTYPE{SCRIPT_RUNTYPE_BREAKONERROR, SCRIPT_RUNTYPE_TRYTOCONTINUE};

	class ScriptRunningSetup{
	public:
		DLLEXPORT ScriptRunningSetup();
		DLLEXPORT ~ScriptRunningSetup();

		// named constructor idiom //
		DLLEXPORT inline ScriptRunningSetup& SetEntrypoint(const string &epoint){
			// set //
			Entryfunction = epoint;
			return *this;
		}
		DLLEXPORT inline ScriptRunningSetup& SetArguements(vector<shared_ptr<NamedVariableBlock>>  &args){
			// set //
			Parameters = args;
			return *this;
		}
		DLLEXPORT inline ScriptRunningSetup& SetUseFullDeclaration(const bool &state){
			// set //
			FullDeclaration = state;
			return *this;
		}
		

		// variables //
		vector<shared_ptr<NamedVariableBlock>> Parameters;

		bool PrintErrors;
		bool FullDeclaration;
		bool ErrorOnNonExistingFunction;
		SCRIPT_RUNTYPE RunType;


		bool ScriptExisted;

		string Entryfunction;
	};

}
#endif