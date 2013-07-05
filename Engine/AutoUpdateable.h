#ifndef LEVIATHAN_AUTOUPDATEABLE
#define LEVIATHAN_AUTOUPDATEABLE
// ------------------------------------ //
#ifndef LEVIATHAN_DEFINE
#include "Define.h"
#endif
// ------------------------------------ //
// ---- includes ---- //
//#include "RenderableGuiObject.h"
#include "NamedVars.h"

namespace Leviathan{

	class AutoUpdateableObject : public Object{
	public:
		DLLEXPORT AutoUpdateableObject::AutoUpdateableObject();
		DLLEXPORT virtual AutoUpdateableObject::~AutoUpdateableObject();

		DLLEXPORT virtual void StartMonitoring(vector<shared_ptr<VariableBlock>> &IndexesAndNamesToListen);
		DLLEXPORT virtual void StopMonitoring(vector<shared_ptr<VariableBlock>> &unregisterindexandnames, bool all = false);

		DLLEXPORT virtual bool OnUpdate(const shared_ptr<NamedVariableList> &updated);



	protected:

		DLLEXPORT void _PopUdated();

		// -------------------------- //
		vector<shared_ptr<VariableBlock>> MonitoredValues;


		bool ValuesUpdated;
		vector<shared_ptr<NamedVariableList>> UpdatedValues;
	};

}
#endif