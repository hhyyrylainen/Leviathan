#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/DataBlock.h"


namespace Leviathan{

	enum SCRIPT_RUNTYPE{SCRIPT_RUNTYPE_BREAKONERROR, SCRIPT_RUNTYPE_TRYTOCONTINUE};

	class ScriptRunningSetup{
	public:
		DLLEXPORT ScriptRunningSetup();
		DLLEXPORT ~ScriptRunningSetup();

		// named constructor idiom //
		DLLEXPORT inline ScriptRunningSetup& SetEntrypoint(const std::string &epoint){
			// set //
			Entryfunction = epoint;
			return *this;
		}
        
		DLLEXPORT inline ScriptRunningSetup& SetArguments(
            std::vector<std::shared_ptr<NamedVariableBlock>> &args)
        {
			// set //
			Parameters = args;
			return *this;
		}
        
		DLLEXPORT inline ScriptRunningSetup& SetUseFullDeclaration(const bool &state){
			// set //
			FullDeclaration = state;
			return *this;
		}
        
        DLLEXPORT inline ScriptRunningSetup& SetPrintErrors(const bool &state){
            
			PrintErrors = state;
			return *this;
		}
		

		// variables //
        std::vector<std::shared_ptr<NamedVariableBlock>> Parameters;

		bool PrintErrors;
		bool FullDeclaration;
		bool ErrorOnNonExistingFunction;
		SCRIPT_RUNTYPE RunType;


		bool ScriptExisted;

        std::string Entryfunction;
	};

}

