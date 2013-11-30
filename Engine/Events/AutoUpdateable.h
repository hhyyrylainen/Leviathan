#ifndef LEVIATHAN_AUTOUPDATEABLE
#define LEVIATHAN_AUTOUPDATEABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
#include "Common\DataStoring\NamedVars.h"
#include "Common\DataStoring\DataBlock.h"

namespace Leviathan{

#define AUTOUPDATEABLEOBJECT_SCRIPTPROXIES DLLEXPORT ScriptSafeVariableBlock* GetAndPopFirstUpdatedProxy(){ return GetAndPopFirstUpdated(); };

	class AutoUpdateableObject{
	public:
		DLLEXPORT AutoUpdateableObject::AutoUpdateableObject();
		DLLEXPORT virtual AutoUpdateableObject::~AutoUpdateableObject();

		DLLEXPORT virtual void StartMonitoring(const vector<VariableBlock*> &IndexesAndNamesToListen);
		DLLEXPORT virtual void StopMonitoring(vector<shared_ptr<VariableBlock>> &unregisterindexandnames, bool all = false);

		DLLEXPORT virtual bool OnUpdate(const shared_ptr<NamedVariableList> &updated);

		DLLEXPORT ScriptSafeVariableBlock* GetAndPopFirstUpdated(){

			auto tmp = new ScriptSafeVariableBlock(UpdatedValues[0]->GetValueDirect(), UpdatedValues[0]->GetName());
			UpdatedValues.erase(UpdatedValues.begin());

			return tmp;
		}

	protected:

		DLLEXPORT void _PopUdated();

		// -------------------------- //
		vector<shared_ptr<VariableBlock>> MonitoredValues;


		bool ValuesUpdated;
		vector<shared_ptr<NamedVariableList>> UpdatedValues;
	};

}
#endif