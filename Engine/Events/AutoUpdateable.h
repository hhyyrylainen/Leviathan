#pragma once
// ------------------------------------ //
#include "Define.h"
// ------------------------------------ //
#include "Common/DataStoring/NamedVars.h"
#include "Common/DataStoring/DataBlock.h"

namespace Leviathan{

#define AUTOUPDATEABLEOBJECT_SCRIPTPROXIES DLLEXPORT ScriptSafeVariableBlock* GetAndPopFirstUpdatedProxy(){ \
        return GetAndPopFirstUpdated(); };

	class AutoUpdateableObject{
	public:
		DLLEXPORT AutoUpdateableObject();
		DLLEXPORT virtual ~AutoUpdateableObject();

		DLLEXPORT virtual void StartMonitoring(
            const std::vector<VariableBlock*> &IndexesAndNamesToListen);
        
		DLLEXPORT virtual void StopMonitoring(
            std::vector<std::shared_ptr<VariableBlock>> &unregisterindexandnames, bool all = false);

		DLLEXPORT virtual bool OnUpdate(const std::shared_ptr<NamedVariableList> &updated);



	protected:

		DLLEXPORT void _PopUdated();

		// -------------------------- //
        std::vector<std::shared_ptr<VariableBlock>> MonitoredValues;


		bool ValuesUpdated;
		std::vector<std::shared_ptr<NamedVariableList>> UpdatedValues;
	};

}

