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

		DLLEXPORT virtual void StartMonitoring(int valueid, bool nonid, wstring varname = L"");
		DLLEXPORT virtual void StopMonitoring(int index, wstring varname = L"", bool all = false);

		DLLEXPORT virtual bool OnUpdate(const shared_ptr<CombinedClass<NamedVar, Int1>> &updated);


		



	protected:

		/*DLLEXPORT*/ void _PopUdated();

		// -------------------------- //
		bool ValuesUpdated;

		vector<int> MonitoredIndexes;
		vector<shared_ptr<wstring>> MonitoredValueNames;
		//vector<shared_ptr<NamedVar>> UpdatedValues;
		vector<shared_ptr<CombinedClass<NamedVar, Int1>>> UpdatedValues;
	};

}
#endif